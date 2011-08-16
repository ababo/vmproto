#include "../common/exception.h"
#include "module.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace Ant::Common;

    const Runtime::ModuleData &Module::moduleData() const {
      const Runtime::ModuleData *data =
        Runtime::instance().findModuleData(_id);

      if(!data)
        throw NotFoundException();

      return *data;
    }

    unsigned int Module::varTypeCount() const {
      return moduleData().vtypes.size();
    }

    unsigned int Module::regCount() const {
      return moduleData().regs.size();
    }

    unsigned int Module::procCount() const {
      return moduleData().procs.size();
    }

    void Module::varTypeById(VarTypeId id, VarType &vtype) const {
      const Runtime::ModuleData &data = moduleData();

      if(id >= data.vtypes.size())
        throw NotFoundException();

      const Runtime::VarTypeData vtypeData = data.vtypes[id];
      vtype.count = vtypeData.count;
      vtype.bytes = vtypeData.bytes;
      vtype.vrefs.assign(vtypeData.vrefs.begin(), vtypeData.vrefs.end());
      vtype.prefs.assign(vtypeData.prefs.begin(), vtypeData.prefs.end());
    }

    VarTypeId Module::regTypeById(RegId id) const {
      const Runtime::ModuleData &data = moduleData();

      if(id < RESERVED_REGS_COUNT)
        throw ArgumentException();
      if(id >= data.regs.size() + RESERVED_REGS_COUNT)
        throw NotFoundException();

      return data.regs[id - RESERVED_REGS_COUNT];
    }

    void Module::procById(ProcId id, Proc &proc) const {
      const Runtime::ModuleData &data = moduleData();

      if(id >= data.procs.size())
        throw NotFoundException();

      const Runtime::ProcData procData = data.procs[id];
      proc.flags = procData.flags;
      proc.io = procData.io;
      proc.code.assign(procData.code.begin(), procData.code.end());
    }

    void Module::drop() {
      moduleData();
      Runtime::instance().dropModuleData(_id);
    }

    void Module::unpack() {

    }

    void Module::callFunc(ProcId func, Variable &io) const {


    }

  }
}
