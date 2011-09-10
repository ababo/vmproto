#ifndef __VM_MDATA_INCLUDED__
#define __VM_MDATA_INCLUDED__

#include "../retained.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    struct Runtime::ModuleData : Retained<ModuleData> {
      struct LLVMContext;

      ModuleData(const UUID &id);

      unsigned int varTypeCount() const;
      unsigned int regCount() const;
      unsigned int procCount() const;

      void varTypeById(VarTypeId id, VarType &vtype) const;
      void regById(RegId id, Reg &reg) const;
      void procById(ProcId id, Proc &proc) const;

      bool isPacked() const;
      bool isDropped() const;

      void pack();
      void unpack();
      void drop();

      void callProc(ProcId proc, Variable &io);

      void take(ModuleData& moduleData);

      void assertNotDropped() const;
      void assertUnpacked() const;

      void prepareLLVMFPM();
      void createLLVMFuncs();
      void prepareLLVMContext(LLVMContext &context);
      void emitLLVMCode(LLVMContext &context);
      template<uint8_t OP, llvm::Instruction::BinaryOps, uint64_t>
        void emitLLVMCodeUO(LLVMContext &context, const UOInstr<OP> &instr);
      template<uint8_t OP, llvm::Instruction::BinaryOps>
        void emitLLVMCodeBO(LLVMContext &context, const BOInstr<OP> &instr);
      void emitLLVMCodeAST(LLVMContext &context, const ASTInstr &instr);
      void emitLLVMCodeFST(LLVMContext &context, const FSTInstr &instr);
      void emitLLVMCodeMOVM8(LLVMContext &context, const MOVM8Instr &instr);
      void emitLLVMCodeMOVN8(LLVMContext &context, const MOVN8Instr &instr);
      void emitLLVMCodeJNZ(LLVMContext &context, const JNZInstr &instr);
      void emitLLVMCodeRET(LLVMContext &context, const RETInstr &instr);
      const llvm::Type *getLLVMTypeById(VarTypeId id) const;

      const UUID &id;
      bool dropped;

      std::vector<VarTypeData> vtypes;
      std::vector<VarTypeId> refs;
      std::vector<Reg> regs;
      std::vector<ProcData> procs;
      std::vector<VMCodeByte> code;

      llvm::Module *llvmModule;
      llvm::FunctionPassManager *llvmFPM;
      llvm::ExecutionEngine *llvmEE;
    };

  }
}

#endif // __VM_MDATA_INCLUDED__
