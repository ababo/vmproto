#include "../exception.h"
#include "instr.h"
#include "llvm/Intrinsics.h"
#include "llvm/Module.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace llvm;

    llvm::Value *&Runtime::ModuleData::LLVMContext::regValue(RegId reg) {
      for(int i = allocs.size() - 1; i >= 0; i--)
        if(allocs[i].reg == reg)
          return allocs[i].value;
      throw BugException();
    }

    void Runtime::ModuleData::LLVMContext::pushAlloc(RegId reg,
                                                     llvm::Value *value) {
      allocs.push_back(Alloc());
      allocs.back().reg = reg;
      allocs.back().value = value;
    }

    void Runtime::ModuleData::LLVMContext::popAlloc() {
      allocs.pop_back();
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
        llvmTypes.clear();
        llvmFuncs.clear();
        delete llvmModule;
        llvmModule = NULL;
      }
    }

    void Runtime::ModuleData::createLLVMTypes() {
      llvmTypes.reserve(vtypes.size());
      for(VarTypeId vtype = 0; vtype < vtypes.size(); vtype++) {
        const Type *byte = IntegerType::get(llvmModule->getContext(), 8);
        llvmTypes.push_back(ArrayType::get(byte, vtypes[vtype].bytes));
      }
    }

    void Runtime::ModuleData::prepareLLVMContext(LLVMContext &context) {
      Instr instr;
      context.blockIndexes.push_back(0);
      for(size_t i = 0, j = 0; j < procs[context.proc].code.size();
          i++, j += instr.size()) {
        instr.set(&procs[context.proc].code[j]);

        if(instr.jumps())
          context.blockIndexes.push_back(instr.jumpIndex(i));
      }
      sort(context.blockIndexes.begin(), context.blockIndexes.end());

      context.blocks.reserve(context.blockIndexes.size());
      for(size_t i = 0; i < context.blockIndexes.size(); i++)
        context.blocks.push_back(BasicBlock::Create(llvmModule->getContext(),
                                                    "", context.func, 0));

      context.pushAlloc(procs[context.proc].io, context.func->arg_begin());
    }

    void Runtime::ModuleData::emitLLVMCodeAST(LLVMContext &context,
                                              const ASTInstr &instr) {
      BasicBlock *block = context.blocks[context.blockIndex];

      Function *ss = Intrinsic::getDeclaration(llvmModule,
                                               Intrinsic::stacksave);
      context.stackPtrs.push_back(CallInst::Create(ss, "", block));

      RegId reg = instr.reg();
      Type * type = llvmTypes[regs[reg]];
      Constant *zeros = ConstantAggregateZero::get(type);
      Value *var = new AllocaInst(type, "", block);
      context.pushAlloc(reg, new StoreInst(var, zeros, block));
    }

    void Runtime::ModuleData::emitLLVMCodeFST(LLVMContext &context,
                                              const FSTInstr &instr) {
      BasicBlock *block = context.blocks[context.blockIndex];

      Function *sr = Intrinsic::getDeclaration(llvmModule,
                                               Intrinsic::stackrestore);
      vector<Value*> args(1, context.stackPtrs.back());
      CallInst::Create(sr, args.begin(), args.end(), "", block);
      context.stackPtrs.pop_back();

      context.popAlloc();
    }

    void Runtime::ModuleData::emitLLVMCodeMOVM8(LLVMContext &context,
                                                const MOVM8Instr &instr) {

    }

    void Runtime::ModuleData::emitLLVMCodeMOVN8(LLVMContext &context,
                                                const MOVN8Instr &instr) {

    }

    void Runtime::ModuleData::emitLLVMCodeUMUL(LLVMContext &context,
                                               const UMULInstr &instr) {

    }

    void Runtime::ModuleData::emitLLVMCodeDEC(LLVMContext &context,
                                              const DECInstr &instr) {

    }

    void Runtime::ModuleData::emitLLVMCodeJNZ(LLVMContext &context,
                                              const JNZInstr &instr) {

    }

    void Runtime::ModuleData::emitLLVMCodeRET(LLVMContext &context,
                                              const RETInstr &instr) {

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
        if(context.blockIndex + 1 < context.blocks.size() &&
           context.blockIndexes[context.blockIndex + 1] == context.instrIndex)
          context.blockIndex++;
      }
    }

    void Runtime::ModuleData::ModuleData::createLLVMFuncs() {
      llvmFuncs.reserve(procs.size());
      for(ProcId proc = 0; proc < procs.size(); proc++) {
        vector<const Type*> argTypes;
        argTypes.push_back(PointerType::get(llvmTypes[procs[proc].io], 0));

        const Type *voidType = Type::getVoidTy(llvmModule->getContext());
        FunctionType *ftype = FunctionType::get(voidType, argTypes, false);

        bool external = procs[proc].flags & PFLAG_EXTERNAL;
        GlobalValue::LinkageTypes link = external ?
          GlobalValue::ExternalLinkage : GlobalValue::InternalLinkage;

        Function *func = Function::Create(ftype, link, "", llvmModule);
        func->setCallingConv(external ? CallingConv::C : CallingConv::Fast);
        llvmFuncs.push_back(func);

        LLVMContext context = { proc, func, 0, 0 };
        prepareLLVMContext(context);
        emitLLVMCode(context);
      }
    }

    void Runtime::ModuleData::unpack() {
      assertNotDropped();

      if(isPacked())
        try {
          llvmModule = new Module("", getGlobalContext());
          createLLVMTypes();
          createLLVMFuncs();
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
      ModuleDataPair p = ModuleDataPair(id, ModuleData());
      ModuleDataIterator i = modules.insert(p).first;
      i->second.take(moduleData);
    }

  }
}
