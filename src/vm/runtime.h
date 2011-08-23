#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <map>
#include <stack>
#include <vector>

#include "../farray.h"
#include "../retained.h"
#include "../singleton.h"
#include "../uuid.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/IRBuilder.h"

namespace Ant {
  namespace VM {

    typedef unsigned int VarTypeId;
    typedef unsigned int ProcId;
    typedef unsigned int RegId;

    typedef unsigned char VMCodeByte;
    typedef const VMCodeByte *VMCode;
    typedef const void *NativeCode;

    struct Variable {};

    template <size_t Count, size_t Bytes, size_t VRefs, size_t PRefs>
    struct StaticVariable : public Variable {
      struct {
        unsigned char bytes[Bytes];
        Variable *vrefs[VRefs];
        NativeCode prefs[PRefs];
      } elts[Count];
    };

    struct VarType {
      size_t count;
      size_t bytes;
      std::vector<VarTypeId> vrefs;
      std::vector<VarTypeId> prefs;
    };

    struct Proc {
      unsigned int flags;
      RegId io;
      std::vector<VMCodeByte> code;
    };

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FUNCTION = 0x2,
      PFLAG_FIRST_RESERVED = 0x4
    };

    class ASTInstr;
    class FSTInstr;
    class MOVM8Instr;
    class MOVN8Instr;
    class UMULInstr;
    class DECInstr;
    class JNZInstr;
    class RETInstr;

    class Runtime : public Singleton<Runtime> {
      friend class Singleton<Runtime>;
      friend class ModuleBuilder;
      friend class Module;
    protected:
      struct VarTypeData {
        size_t count;
        size_t bytes;
        FixedArray<VarTypeId> vrefs;
        FixedArray<VarTypeId> prefs;
      };
      struct ProcData {
        unsigned int flags;
        RegId io;
        FixedArray<VMCodeByte> code;
      };
      struct ModuleData : Retained<ModuleData> {
        struct LLVMContext {
          typedef struct { RegId reg; llvm::Value *val; } _;
          typedef std::stack<llvm::Value*> RegState;
          typedef std::map<RegId, RegState> RegMap;
          typedef RegMap::value_type RegMapPair;
          typedef RegMap::iterator RegMapIterator;
          typedef RegMap::const_iterator RegMapConstIterator;

          llvm::Value *regValue(RegId reg) const;
          void pushRegValue(RegId reg, llvm::Value *val);
          void popRegValue(RegId reg);

          ProcId proc;
          llvm::Function *func;
          size_t instrIndex, blockIndex;
          std::vector<size_t> blockIndexes;
          std::vector<llvm::BasicBlock*> blocks;
          std::stack<llvm::Value*> stackPtrs;
          RegMap regStates;
        };

        ModuleData() : dropped(false), llvmModule(NULL) {}

        unsigned int varTypeCount() const;
        unsigned int regCount() const;
        unsigned int procCount() const;

        void varTypeById(VarTypeId id, VarType &vtype) const;
        VarTypeId regTypeById(RegId id) const;
        void procById(ProcId id, Proc &proc) const;

        bool isPacked() const;
        bool isDropped() const;

        void pack();
        void unpack();
        void drop();

        void take(ModuleData& moduleData);

        void assertNotDropped() const;
        void assertUnpacked() const;

        void createLLVMTypes();
        void createLLVMFuncs();
        void prepareLLVMContext(LLVMContext &context);
        void emitLLVMCode(LLVMContext &context);
        void emitLLVMCodeAST(LLVMContext &context, const ASTInstr &instr);
        void emitLLVMCodeFST(LLVMContext &context, const FSTInstr &instr);
        void emitLLVMCodeMOVM8(LLVMContext &context, const MOVM8Instr &instr);
        void emitLLVMCodeMOVN8(LLVMContext &context, const MOVN8Instr &instr);
        void emitLLVMCodeUMUL(LLVMContext &context, const UMULInstr &instr);
        void emitLLVMCodeDEC(LLVMContext &context, const DECInstr &instr);
        void emitLLVMCodeJNZ(LLVMContext &context, const JNZInstr &instr);
        void emitLLVMCodeRET(LLVMContext &context, const RETInstr &instr);

        bool dropped;

        std::vector<VarTypeData> vtypes;
        std::vector<VarTypeId> refs;
        std::vector<VarTypeId> regs;
        std::vector<ProcData> procs;
        std::vector<VMCodeByte> code;

        llvm::Module *llvmModule;
        std::vector<llvm::Type*> llvmTypes;
        std::vector<llvm::Function*> llvmFuncs;
      };
      typedef std::map<UUID, ModuleData> ModuleDataMap;
      typedef ModuleDataMap::value_type ModuleDataPair;
      typedef ModuleDataMap::iterator ModuleDataIterator;

      ModuleDataIterator retainModuleData(const UUID &id);
      void releaseModuleData(ModuleDataIterator moduleDataIter);
      void insertModuleData(const UUID &id, ModuleData &moduleData);

      ModuleDataMap modules;
      llvm::IRBuilder<> llvmBuilder;

    private:
      Runtime() : Singleton<Runtime>(0),
                  llvmBuilder(llvm::getGlobalContext()) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
