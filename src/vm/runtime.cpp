#include "../exception.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

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

    void Runtime::ModuleData::take(ModuleData& moduleData) {
      vtypes.swap(moduleData.vtypes);
      refs.swap(moduleData.refs);
      regs.swap(moduleData.regs);
      procs.swap(moduleData.procs);
      code.swap(moduleData.code);
    }

    Runtime::ModuleDataIterator Runtime::retainModuleData(const UUID &id) {
      ModuleDataIterator i = modules.find(id);

      if(i != modules.end() && !i->second.dropped) {
        i->second.retain();
        return i;
      }

      return modules.end();
    }

    void Runtime::releaseModuleData(ModuleDataIterator moduleDataIter) {
      ModuleData &data = moduleDataIter->second;

      data.release();
      if(data.retainCount() == 1 && data.dropped)
        modules.erase(moduleDataIter);
    }

    void Runtime::insertModuleData(const UUID &id, ModuleData &moduleData) {
      ModuleDataPair p = ModuleDataPair(id, ModuleData());
      ModuleDataIterator i = modules.insert(p).first;
      i->second.take(moduleData);
    }

    void Runtime::dropModuleData(ModuleDataIterator moduleDataIter) {
      moduleDataIter->second.assertNotDropped();
      moduleDataIter->second.dropped = true;
    }

  }
}
