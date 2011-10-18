#ifndef __VM_MDATA_INCLUDED__
#define __VM_MDATA_INCLUDED__

#include <stdint.h>

#include "../retained.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    struct Runtime::ModuleData : Retained<ModuleData> {
      struct LLVMContext;
      enum SpeField { SFLD_REF_COUNT, SFLD_ELT_COUNT };
      enum EltField { EFLD_BYTES, EFLD_VREFS, EFLD_PREFS };

      ModuleData(const UUID &id);

      uint32_t varTypeCount() const;
      uint32_t procTypeCount() const;
      uint32_t regCount() const;
      uint32_t procCount() const;

      void varTypeById(VarTypeId id, VarType &vtype) const;
      void procTypeById(ProcTypeId id, ProcType &ptype) const;
      void regById(RegId id, VarSpec &reg) const;
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
      void createLLVMPVars();
      void createLLVMFuncs();
      void prepareLLVMContext(LLVMContext &context);
      void emitLLVMCode(LLVMContext &context);
      const llvm::Type *getEltLLVMType(VarTypeId vtype) const;
      llvm::Value *regValue(LLVMContext &context, RegId reg,
                            bool dereferenceIfNeeded = true);
      llvm::Value *specialPtr(LLVMContext &context, llvm::Value *vptr,
                              SpeField sfld, bool fixed);
      llvm::Value *elementPtr(LLVMContext &context, llvm::Value *vptr,
                          llvm::Value *elti = NULL, EltField efld = EFLD_BYTES,
                              uint32_t subi = 0);
      llvm::Value *stackAlloc(LLVMContext &context, RegId reg, bool ref,
                              llvm::Value *eltCount = NULL);
      template<uint8_t OP, llvm::Instruction::BinaryOps, uint64_t>
        void emitLLVMCodeUO(LLVMContext &context, const UOInstrT<OP> &instr);
      template<uint8_t OP, llvm::Instruction::BinaryOps>
        void emitLLVMCodeBO(LLVMContext &context, const BOInstrT<OP> &instr);
      template<uint8_t OP, llvm::ICmpInst::Predicate, uint64_t>
        void emitLLVMCodeUJ(LLVMContext &context, const UJInstrT<OP> &instr);
      template<uint8_t OP, llvm::ICmpInst::Predicate>
        void emitLLVMCodeBJ(LLVMContext &context, const BJInstrT<OP> &instr);
      template<uint8_t OP, class VAL>
        void emitLLVMCodeCPI(LLVMContext &context,
                             const CPIInstrT<OP, VAL> &instr);
      template<uint8_t OP, bool REF>
        void emitLLVMCodePUSH(LLVMContext &context,
                              const PUSHInstrT<OP, REF> &instr);
      void emitLLVMCodePUSHH(LLVMContext &context, const PUSHHInstr &instr);
      void emitLLVMCodePOP(LLVMContext &context, const POPInstr &instr);
      void emitLLVMCodePOPL(LLVMContext &context, const POPLInstr &instr);
      void emitLLVMCodeJMP(LLVMContext &context, const JMPInstr &instr);
      void emitLLVMCodeCPB(LLVMContext &context, const CPBInstr &instr);
      void emitLLVMCodeLDE(LLVMContext &context, const LDEInstr &instr);
      void emitLLVMCodeLDB(LLVMContext &context, const LDBInstr &instr);
      void emitLLVMCodeLDR(LLVMContext &context, const LDRInstr &instr);
      void emitLLVMCodeSTE(LLVMContext &context, const STEInstr &instr);
      void emitLLVMCodeSTB(LLVMContext &context, const STBInstr &instr);
      void emitLLVMCodeCALL(LLVMContext &context, const CALLInstr &instr);
      void emitLLVMCodeTHROW(LLVMContext &context, const THROWInstr &instr);
      void emitLLVMCodeRET(LLVMContext &context, const RETInstr &instr);

      const UUID &id;
      bool dropped;

      std::vector<VarTypeData> vtypes;
      std::vector<ProcType> ptypes;
      std::vector<VarSpec> vrefs;
      std::vector<ProcTypeId> prefs;
      std::vector<VarSpec> regs;
      std::vector<ProcData> procs;
      std::vector<VMCodeByte> code;

      llvm::Module *llvmModule;
      llvm::FunctionPassManager *llvmFPM;
      llvm::ExecutionEngine *llvmEE;
    };

  }
}

#endif // __VM_MDATA_INCLUDED__
