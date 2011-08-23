#include "../exception.h"
#include "llvm/Module.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace llvm;

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

    void Runtime::ModuleData::createLLVMFuncs() {
      llvmFuncs.reserve(procs.size());
      for(ProcId proc = 0; proc < procs.size(); proc++) {
        vector<const Type*> argTypes;
        argTypes.push_back(llvmTypes[procs[proc].io]);
        const Type *voidType = Type::getVoidTy(llvmModule->getContext());
        FunctionType *ftype = FunctionType::get(voidType, argTypes, false);

        GlobalValue::LinkageTypes link = procs[proc].flags & PFLAG_EXTERNAL ?
          GlobalValue::ExternalLinkage : GlobalValue::InternalLinkage;
        Function *func = Function::Create(ftype, link, "", llvmModule);
        func->setCallingConv(CallingConv::C);
        llvmFuncs.push_back(func);

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
