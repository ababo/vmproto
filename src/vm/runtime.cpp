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
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace llvm;

    Value *Runtime::ModuleData::LLVMContext::sptr() {
      return allocs.back().sptr;
    }

    Value *Runtime::ModuleData::LLVMContext::vptr(RegId reg) {
      for(int i = allocs.size() - 1; i >= 0; i--)
        if(allocs[i].reg == reg)
          return allocs[i].vptr;

      throw BugException();
    }

    void Runtime::ModuleData::LLVMContext::pushAlloc(RegId reg,
                                                     llvm::Value *sptr,
                                                     llvm::Value *vptr) {
      allocs.push_back(Alloc());
      allocs.back().reg = reg;
      allocs.back().sptr = sptr;
      allocs.back().vptr = vptr;
    }

    void Runtime::ModuleData::LLVMContext::popAlloc() {
      allocs.pop_back();
    }

    BasicBlock *Runtime::ModuleData::LLVMContext::jumpBlock(size_t jumpIndex) {
      vector<size_t>::iterator iter = lower_bound(blockIndexes.begin(),
                                                  blockIndexes.end(),
                                                  jumpIndex);
      if(iter == blockIndexes.end())
        throw BugException();

      return blocks[distance(blockIndexes.begin(), iter)];
    }

    void Runtime::ModuleData::assertNotDropped() const {
      if(dropped)
        throw NotFoundException();
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
      vtype.count = vtypeData.count;
      vtype.bytes = vtypeData.bytes;
      vtype.vrefs.assign(vtypeData.vrefs.begin(), vtypeData.vrefs.end());
      vtype.prefs.assign(vtypeData.prefs.begin(), vtypeData.prefs.end());
    }

    VarTypeId Runtime::ModuleData::regTypeById(RegId id) const {
      assertNotDropped();

      if(id >= regs.size())
        throw NotFoundException();

      return regs[id];
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

    const Type *Runtime::ModuleData::getLLVMTypeById(VarTypeId id) const {
      return ArrayType::get(TYPE_INT(8), vtypes[id].bytes);
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

    void Runtime::ModuleData::emitLLVMCodeAST(LLVMContext &context,
                                              const ASTInstr &instr) {
      Function *ss = Intrinsic::getDeclaration(llvmModule,
                                               Intrinsic::stacksave);
      RegId reg = instr.reg();
      const Type *type = getLLVMTypeById(regs[reg]);
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

#define CONST_I64(val, signed) \
    ConstantInt::get(llvmModule->getContext(), APInt(64, val, signed))
#define TYPE_PTR(type) PointerType::get(type, 0)
#define BITCAST_PI64(ptr) \
    new BitCastInst(ptr, TYPE_PTR(TYPE_INT(64)), "", CURRENT_BLOCK)

    void Runtime::ModuleData::emitLLVMCodeMOVM8(LLVMContext &context,
                                                const MOVM8Instr &instr) {
      Value *to = BITCAST_PI64(context.vptr(instr.to()));
      new StoreInst(CONST_I64(instr.val(), false), to, CURRENT_BLOCK);
    }

    void Runtime::ModuleData::emitLLVMCodeMOVN8(LLVMContext &context,
                                                const MOVN8Instr &instr) {
      Value *from = BITCAST_PI64(context.vptr(instr.from()));
      Value *to = BITCAST_PI64(context.vptr(instr.to()));
      Value *val = new LoadInst(from, "", CURRENT_BLOCK);
      new StoreInst(val, to, CURRENT_BLOCK);
    }

#define BIN_OP(op, val1, val2) \
    BinaryOperator::Create(Instruction::op, val1, val2, "", CURRENT_BLOCK)

    void Runtime::ModuleData::emitLLVMCodeUMUL(LLVMContext &context,
                                               const UMULInstr &instr) {
      Value *factor1 = BITCAST_PI64(context.vptr(instr.factor1()));
      Value *factor2 = BITCAST_PI64(context.vptr(instr.factor2()));
      Value *product = BITCAST_PI64(context.vptr(instr.product()));
      Value *val1 = new LoadInst(factor1, "", CURRENT_BLOCK);
      Value *val2 = new LoadInst(factor2, "", CURRENT_BLOCK);
      Value *val3 = BIN_OP(Mul, val1, val2);
      new StoreInst(val3, product, CURRENT_BLOCK);
    }

    void Runtime::ModuleData::emitLLVMCodeDEC(LLVMContext &context,
                                              const DECInstr &instr) {
      Value *it = BITCAST_PI64(context.vptr(instr.it()));
      Value *val = new LoadInst(it, "", CURRENT_BLOCK);
      val = BIN_OP(Sub, val, CONST_I64(1, false));
      new StoreInst(val, it, CURRENT_BLOCK);
    }

    void Runtime::ModuleData::emitLLVMCodeJNZ(LLVMContext &context,
                                              const JNZInstr &instr) {
      Value *it = BITCAST_PI64(context.vptr(instr.it()));
      Value *val = new LoadInst(it, "", CURRENT_BLOCK);
      ICmpInst* cmp = new ICmpInst(*CURRENT_BLOCK, ICmpInst::ICMP_NE,
                                   val, CONST_I64(0, false));
      size_t jindex = instr.jumpIndex(context.instrIndex);
      BasicBlock *jblock = context.jumpBlock(jindex);
      BasicBlock *nblock = context.blocks[context.blockIndex + 1];
      BranchInst::Create(jblock, nblock, cmp, CURRENT_BLOCK);
    }

    void Runtime::ModuleData::emitLLVMCodeRET(LLVMContext &context,
                                              const RETInstr &instr) {
      ReturnInst::Create(llvmModule->getContext(), CURRENT_BLOCK);
    }

#define INSTR_CASE(op) \
    case OPCODE_##op: \
      emitLLVMCode##op(context, static_cast<op##Instr&>(instr)); break;

    void Runtime::ModuleData::emitLLVMCode(LLVMContext &context) {
      Instr instr;
      for(size_t i = 0; i < procs[context.proc].code.size();
          i += instr.size()) {
        instr.set(&procs[context.proc].code[i]);

        switch(instr.opcode()) {
          INSTR_CASE(AST);
          INSTR_CASE(FST);
          INSTR_CASE(MOVM8);
          INSTR_CASE(MOVN8);
          INSTR_CASE(UMUL);
          INSTR_CASE(DEC);
          INSTR_CASE(JNZ);
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
          llvmModule = new Module(idStr, getGlobalContext());
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

    }

    void Runtime::ModuleData::take(ModuleData& moduleData) {
      vtypes.swap(moduleData.vtypes);
      refs.swap(moduleData.refs);
      regs.swap(moduleData.regs);
      procs.swap(moduleData.procs);
      code.swap(moduleData.code);
    }

    Runtime::ModuleDataIterator Runtime::retainModuleData(const UUID &id) {
      ModuleDataIterator i = modules.find(id);

      if(i != modules.end() && !i->second.isDropped()) {
        i->second.retain();
        return i;
      }

      return modules.end();
    }

    void Runtime::releaseModuleData(ModuleDataIterator moduleDataIter) {
      ModuleData &data = moduleDataIter->second;

      data.release();
      if(data.retainCount() == 1 && data.isDropped())
        modules.erase(moduleDataIter);
    }

    void Runtime::insertModuleData(const UUID &id, ModuleData &moduleData) {
      ModuleDataPair p = ModuleDataPair(id, ModuleData(id));
      ModuleDataIterator i = modules.insert(p).first;
      i->second.take(moduleData);
    }

  }
}
