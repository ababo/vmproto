#include <iostream>
#include <set>
#include <sstream>

#include "../exception.h"
#include "instr.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Constants.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LLVMContext.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Intrinsics.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "mdata.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace llvm;

    struct Runtime::ModuleData::LLVMContext {
      struct Alloc {
        RegId reg;
        llvm::Value *sptr;
        llvm::Value *vptr;
      };

      llvm::Value *sptr() { return allocs.back().sptr; }
      llvm::Value *vptr(RegId reg) {
        for(int i = allocs.size() - 1; i >= 0; i--)
          if(allocs[i].reg == reg)
            return allocs[i].vptr;
        throw BugException();
      }
      void pushAlloc(RegId reg, llvm::Value *sptr, llvm::Value *vptr) {
        allocs.push_back(Alloc());
        allocs.back().reg = reg;
        allocs.back().sptr = sptr;
        allocs.back().vptr = vptr;
      }
      void popAlloc() { allocs.pop_back(); }

      llvm::BasicBlock *jumpBlock(size_t jumpIndex) {
        vector<size_t>::iterator iter = lower_bound(blockIndexes.begin(),
                                                    blockIndexes.end(),
                                                    jumpIndex);
        if(iter == blockIndexes.end())
          throw BugException();
        return blocks[distance(blockIndexes.begin(), iter)];
      }

      ProcId proc;
      llvm::Function *func;
      size_t instrIndex, blockIndex;
      std::vector<size_t> blockIndexes;
      std::vector<llvm::BasicBlock*> blocks;
      std::vector<Alloc> allocs;
    };

    Runtime::ModuleData::ModuleData(const UUID &id)
      : id(id), dropped(false), llvmModule(NULL), llvmFPM(NULL), llvmEE(NULL) {
      llvm::InitializeNativeTarget();
    }

    void Runtime::ModuleData::assertNotDropped() const {
      if(isDropped())
        throw NotFoundException();
    }

    void Runtime::ModuleData::assertUnpacked() const {
      assertNotDropped();

      if(isPacked())
        throw OperationException();
    }

    unsigned int Runtime::ModuleData::varTypeCount() const {
      assertNotDropped();
      return vtypes.size();
    }

    unsigned int Runtime::ModuleData::regCount() const {
      assertNotDropped();
      return regs.size();
    }
    
    unsigned int Runtime::ModuleData::procCount() const {
      assertNotDropped();
      return procs.size();
    }

    void Runtime::ModuleData::varTypeById(VarTypeId id, VarType &vtype) const {
      assertNotDropped();

      if(id >= vtypes.size())
        throw NotFoundException();

      const VarTypeData vtypeData = vtypes[id];
      vtype.bytes = vtypeData.bytes;
      vtype.vrefs.assign(vtypeData.vrefs.begin(), vtypeData.vrefs.end());
      vtype.prefs.assign(vtypeData.prefs.begin(), vtypeData.prefs.end());
    }

    void Runtime::ModuleData::regById(RegId id, Reg &reg) const {
      assertNotDropped();

      if(id >= regs.size())
        throw NotFoundException();

      reg = regs[id];
    }

    void Runtime::ModuleData::procById(ProcId id, Proc &proc) const {
      assertNotDropped();

      if(id >= procs.size())
        throw NotFoundException();

      const ProcData procData = procs[id];
      proc.flags = procData.flags;
      proc.io = procData.io;
      proc.code.assign(procData.code.begin(), procData.code.end());
    }

    bool Runtime::ModuleData::isPacked() const {
      assertNotDropped();
      return !llvmModule;
    }

    bool Runtime::ModuleData::isDropped() const {
      return dropped;
    }

    void Runtime::ModuleData::pack() {
      assertNotDropped();

      if(llvmModule) {
        delete llvmEE;
        llvmEE = NULL;
        delete llvmFPM;
        llvmFPM = NULL;
        // llvmEE already deleted llvmModule
        llvmModule = NULL;
      }
    }

#define TYPE_INT(bits) IntegerType::get(llvmModule->getContext(), bits)
#define TYPE_ARR(bytes) ArrayType::get(TYPE_INT(8), bytes)

    const Type *Runtime::ModuleData::getLLVMTypeById(VarTypeId id) const {
      return TYPE_ARR(vtypes[id].bytes);
    }

    void Runtime::ModuleData::prepareLLVMContext(LLVMContext &context) {
      bool prevBreaks = false;
      set<size_t> indexes;
      Instr instr;
      indexes.insert(0);
      for(size_t i = 0, j = 0; j < procs[context.proc].code.size();
          i++, j += instr.size()) {
        if(prevBreaks) {
          indexes.insert(i);
          prevBreaks = false;
        }

        instr.set(&procs[context.proc].code[j]);
        if(instr.jumps()) {
          indexes.insert(i + 1);
          indexes.insert(instr.jumpIndex(i));
        }
        else prevBreaks = instr.breaks();
      }
      context.blockIndexes.assign(indexes.begin(), indexes.end());
      sort(context.blockIndexes.begin(), context.blockIndexes.end());

      context.blocks.reserve(context.blockIndexes.size());
      for(size_t i = 0; i < context.blockIndexes.size(); i++)
        context.blocks.push_back(BasicBlock::Create(llvmModule->getContext(),
                                                    "", context.func, 0));

      context.pushAlloc(procs[context.proc].io, NULL,
                        context.func->arg_begin());
    }

#define CURRENT_BLOCK context.blocks[context.blockIndex]
#define CONST_INT(bits, val, signed) \
    ConstantInt::get(llvmModule->getContext(), APInt(bits, val, signed))
#define TYPE_PTR(type) PointerType::get(type, 0)
#define BITCAST_PINT(bits, ptr) \
    new BitCastInst(ptr, TYPE_PTR(TYPE_INT(bits)), "", CURRENT_BLOCK)

    template<uint8_t OP, Instruction::BinaryOps IOP, uint64_t CO>
      void Runtime::ModuleData::emitLLVMCodeUO(LLVMContext &context,
                                               const UOInstr<OP> &instr) {
      Value *it = BITCAST_PINT(64, context.vptr(instr.it()));
      Value *val = new LoadInst(it, "", CURRENT_BLOCK);
      Value *co = CONST_INT(64, CO, false);
      val = BinaryOperator::Create(IOP, val, co, "", CURRENT_BLOCK);
      new StoreInst(val, it, CURRENT_BLOCK);
    }

    template<uint8_t OP, Instruction::BinaryOps IOP>
      void Runtime::ModuleData::emitLLVMCodeBO(LLVMContext &context,
                                               const BOInstr<OP> &instr) {
      Value *operand1 = BITCAST_PINT(64, context.vptr(instr.operand1()));
      Value *operand2 = BITCAST_PINT(64, context.vptr(instr.operand2()));
      Value *result = BITCAST_PINT(64, context.vptr(instr.result()));
      Value *val1 = new LoadInst(operand1, "", CURRENT_BLOCK);
      Value *val2 = new LoadInst(operand2, "", CURRENT_BLOCK);
      Value *val3 = BinaryOperator::Create(IOP, val1, val2, "", CURRENT_BLOCK);
      new StoreInst(val3, result, CURRENT_BLOCK);
    }

    template<uint8_t OP, class VAL>
      void Runtime::ModuleData::emitLLVMCodeIMM(LLVMContext &context,
                                              const IMMInstr<OP, VAL> &instr) {
      Value *to = BITCAST_PINT(sizeof(VAL), context.vptr(instr.to()));
      new StoreInst(CONST_INT(sizeof(VAL), uint64_t(instr.val()), false), to,
                    CURRENT_BLOCK);
    }

    template<uint8_t OP, llvm::ICmpInst::Predicate PR, uint64_t CO>
      void Runtime::ModuleData::emitLLVMCodeUJ(LLVMContext &context,
                                               const UJInstr<OP> &instr) {
      Value *it = BITCAST_PINT(64, context.vptr(instr.it()));
      Value *val = new LoadInst(it, "", CURRENT_BLOCK);
      ICmpInst* cmp = new ICmpInst(*CURRENT_BLOCK, PR, val,
                                   CONST_INT(64, CO, false));
      size_t jindex = instr.jumpIndex(context.instrIndex);
      BasicBlock *jblock = context.jumpBlock(jindex);
      BasicBlock *nblock = context.blocks[context.blockIndex + 1];
      BranchInst::Create(jblock, nblock, cmp, CURRENT_BLOCK);
    }

    void Runtime::ModuleData::emitLLVMCodeAST(LLVMContext &context,
                                              const ASTInstr &instr) {
      Function *ss = Intrinsic::getDeclaration(llvmModule,
                                               Intrinsic::stacksave);
      RegId reg = instr.reg();
      const Type *type = getLLVMTypeById(regs[reg].vtype);
      Constant *zeros = ConstantAggregateZero::get(type);
      Value *sptr = CallInst::Create(ss, "", CURRENT_BLOCK);
      Value *vptr = new AllocaInst(type, "", CURRENT_BLOCK);
      new StoreInst(zeros, vptr, CURRENT_BLOCK);
      context.pushAlloc(reg, sptr, vptr);
    }

    void Runtime::ModuleData::emitLLVMCodeFST(LLVMContext &context,
                                              const FSTInstr &instr) {
      Function *sr = Intrinsic::getDeclaration(llvmModule,
                                               Intrinsic::stackrestore);
      vector<Value*> args(1, context.sptr());
      CallInst::Create(sr, args.begin(), args.end(), "", CURRENT_BLOCK);
      context.popAlloc();
    }

#define BITCAST_PARR(bytes, ptr) \
    new BitCastInst(ptr, TYPE_PTR(TYPE_ARR(bytes)), "", CURRENT_BLOCK)

    void Runtime::ModuleData::emitLLVMCodeCPB(LLVMContext &context,
                                              const CPBInstr &instr) {
      RegId f = instr.from(), t = instr.to();
      uint32_t fbytes = vtypes[regs[f].vtype].bytes;
      uint32_t tbytes = vtypes[regs[t].vtype].bytes;
      uint32_t mbytes = fbytes < tbytes ? fbytes : tbytes;
      Value *from = BITCAST_PARR(mbytes, context.vptr(f));
      Value *to = BITCAST_PARR(mbytes, context.vptr(t));
      Value *val = new LoadInst(from, "", CURRENT_BLOCK);
      new StoreInst(val, to, CURRENT_BLOCK);
    }

    void Runtime::ModuleData::emitLLVMCodeRET(LLVMContext &context,
                                              const RETInstr &instr) {
      ReturnInst::Create(llvmModule->getContext(), CURRENT_BLOCK);
    }

#define UOINSTR_CASE(op, iop, co) \
    case OPCODE_##op: \
      emitLLVMCodeUO<OPCODE_##op, Instruction::iop, co>( \
        context, static_cast<UOInstr<OPCODE_##op>&>(instr)); break;

#define BOINSTR_CASE(op, iop) \
    case OPCODE_##op: \
      emitLLVMCodeBO<OPCODE_##op, Instruction::iop>( \
        context, static_cast<BOInstr<OPCODE_##op>&>(instr)); break;

#define IMMINSTR_CASE(op, val) \
    case OPCODE_##op: \
      emitLLVMCodeIMM<OPCODE_##op, val>( \
        context, static_cast<IMMInstr<OPCODE_##op, val>&>(instr)); break;

#define INSTR_CASE(op) \
    case OPCODE_##op: \
      emitLLVMCode##op(context, static_cast<op##Instr&>(instr)); break;

#define UJINSTR_CASE(op, pr, co) \
    case OPCODE_##op: \
      emitLLVMCodeUJ<OPCODE_##op, ICmpInst::pr, co>( \
        context, static_cast<UJInstr<OPCODE_##op>&>(instr)); break;

    void Runtime::ModuleData::emitLLVMCode(LLVMContext &context) {
      Instr instr;
      for(size_t i = 0; i < procs[context.proc].code.size();
          i += instr.size()) {
        instr.set(&procs[context.proc].code[i]);

        switch(instr.opcode()) {
          UOINSTR_CASE(INC, Add, 1);
          UOINSTR_CASE(DEC, Sub, 1);
          BOINSTR_CASE(ADD, Add);
          BOINSTR_CASE(SUB, Sub);
          BOINSTR_CASE(MUL, Mul);
          IMMINSTR_CASE(IMM1, uint8_t);
          IMMINSTR_CASE(IMM2, uint16_t);
          IMMINSTR_CASE(IMM4, uint32_t);
          IMMINSTR_CASE(IMM8, uint64_t);
          UJINSTR_CASE(JNZ, ICMP_NE, 0);
          INSTR_CASE(AST);
          INSTR_CASE(FST);
          INSTR_CASE(CPB);
          INSTR_CASE(RET);
        }

        context.instrIndex++;
        size_t nextBlockIndex = context.blockIndex + 1;
        if(nextBlockIndex < context.blocks.size() &&
           context.blockIndexes[nextBlockIndex] == context.instrIndex) {
          BasicBlock *block = context.blocks[context.blockIndex];
          if(!block->getTerminator())
            BranchInst::Create(context.blocks[nextBlockIndex], block);
          context.blockIndex++;
        }
      }

      if(!context.blocks.back()->getTerminator())
        ReturnInst::Create(llvmModule->getContext(), context.blocks.back());
    }

    void Runtime::ModuleData::createLLVMFuncs() {
      for(ProcId proc = 0; proc < procs.size(); proc++) {
        vector<const Type*> argTypes;
        argTypes.push_back(TYPE_PTR(getLLVMTypeById(procs[proc].io)));
        const Type *voidType = Type::getVoidTy(llvmModule->getContext());
        FunctionType *ftype = FunctionType::get(voidType, argTypes, false);

        bool external = procs[proc].flags & PFLAG_EXTERNAL;
        GlobalValue::LinkageTypes link = external ?
          GlobalValue::ExternalLinkage : GlobalValue::InternalLinkage;

        ostringstream out;
        out << proc;

        Function *func = Function::Create(ftype, link, out.str(), llvmModule);
        func->setCallingConv(external ? CallingConv::C : CallingConv::Fast);

        LLVMContext context = { proc, func, 0, 0 };
        prepareLLVMContext(context);
        emitLLVMCode(context);

#ifdef CONFIG_DEBUG
        func->dump();
        llvmFPM->run(*func);
        func->dump();
#else
        llvmFPM->run(*func);
#endif
      }
    }

    void Runtime::ModuleData::prepareLLVMFPM() {
      llvmFPM->add(new TargetData(*llvmEE->getTargetData()));
      llvmFPM->add(createBasicAliasAnalysisPass());
      llvmFPM->add(createInstructionCombiningPass());
      llvmFPM->add(createReassociatePass());
      llvmFPM->add(createGVNPass());
      llvmFPM->add(createCFGSimplificationPass());

      llvmFPM->doInitialization();
    }

    void Runtime::ModuleData::unpack() {
      assertNotDropped();

      if(isPacked())
        try {
          string err, idStr = id.str();
          llvmModule = new llvm::Module(idStr, getGlobalContext());
          llvmFPM = new FunctionPassManager(llvmModule);
          llvmEE = EngineBuilder(llvmModule).setErrorStr(&err).create();
          if(!llvmEE) {
#ifdef CONFIG_DEBUG
            cerr << endl << "Cannot load JIT (" << err << ")" << endl << endl;
#endif
            throw EnvironmentException();
          }

          prepareLLVMFPM();
          createLLVMFuncs();

#ifdef CONFIG_DEBUG
          if(verifyModule(*llvmModule, PrintMessageAction))
            throw BugException();
#endif
        }
        catch(...) { pack(); throw; }
    }

    void Runtime::ModuleData::drop() {
      assertNotDropped();

      pack();

      vtypes.clear();
      refs.clear();
      regs.clear();
      procs.clear();
      code.clear();

      dropped = true;
    }

    void Runtime::ModuleData::callProc(ProcId proc, Variable &io) {
      assertUnpacked();

      ostringstream out;
      out << proc;
      Function *func = llvmModule->getFunction(out.str());
      if(!func)
        throw NotFoundException();

      void *vPtr = llvmEE->getPointerToFunction(func);
      uintptr_t uPtr = reinterpret_cast<uintptr_t>(vPtr);
      reinterpret_cast<void (*)(Variable&)>(uPtr)(io);
    }

    void Runtime::ModuleData::take(ModuleData& moduleData) {
      vtypes.swap(moduleData.vtypes);
      refs.swap(moduleData.refs);
      regs.swap(moduleData.regs);
      procs.swap(moduleData.procs);
      code.swap(moduleData.code);
    }

  }
}
