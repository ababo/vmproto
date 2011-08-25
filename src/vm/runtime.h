#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <map>
#include <vector>

#include "../farray.h"
#include "../retained.h"
#include "../singleton.h"
#include "../uuid.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetSelect.h"

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
          struct Alloc {
            RegId reg;
            llvm::Value *sptr;
            llvm::Value *vptr;
          };

          llvm::Value *sptr();
          llvm::Value *vptr(RegId reg);
          void pushAlloc(RegId reg, llvm::Value *sptr, llvm::Value *vptr);
          void popAlloc();

          llvm::BasicBlock *jumpBlock(size_t jumpIndex);

          ProcId proc;
          llvm::Function *func;
          size_t instrIndex, blockIndex;
          std::vector<size_t> blockIndexes;
          std::vector<llvm::BasicBlock*> blocks;
          std::vector<Alloc> allocs;
        };

        ModuleData(const UUID &id) : id(id), dropped(false), llvmModule(NULL),
                                     llvmFPM(NULL), llvmEE(NULL) {}

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

        void callProc(ProcId proc, Variable &io);

        void take(ModuleData& moduleData);

        void assertNotDropped() const;
        void assertUnpacked() const;

        void prepareLLVMFPM();
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
        const llvm::Type *getLLVMTypeById(VarTypeId id) const;

        const UUID &id;
        bool dropped;

        std::vector<VarTypeData> vtypes;
        std::vector<VarTypeId> refs;
        std::vector<VarTypeId> regs;
        std::vector<ProcData> procs;
        std::vector<VMCodeByte> code;

        llvm::Module *llvmModule;
        llvm::FunctionPassManager *llvmFPM;
        llvm::ExecutionEngine *llvmEE;
      };
      typedef std::map<UUID, ModuleData> ModuleDataMap;
      typedef ModuleDataMap::value_type ModuleDataPair;
      typedef ModuleDataMap::iterator ModuleDataIterator;

      ModuleDataIterator retainModuleData(const UUID &id);
      void releaseModuleData(ModuleDataIterator moduleDataIter);
      void insertModuleData(const UUID &id, ModuleData &moduleData);

      ModuleDataMap modules;

    private:
      Runtime() : Singleton<Runtime>(0) { llvm::InitializeNativeTarget(); }
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
