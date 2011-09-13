#include "../exception.h"
#include "mdata.h"
#include "module.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace Ant;

    void Module::id(const UUID &id) {
      _id = id;

      Runtime &rt = Runtime::instance();
      if(iter != rt.modules.end()) {
        rt.releaseModuleData(iter);
        iter = rt.modules.end();
      }
    }

    Runtime::ModuleData &Module::moduleData() const {
      Runtime &rt = Runtime::instance();

      if(iter == rt.modules.end()) {
        iter = rt.retainModuleData(_id);

        if(iter == rt.modules.end())
          throw NotFoundException();
      }        

      return *iter->second;
    }

    uint32_t Module::varTypeCount() const {
      return moduleData().varTypeCount();
    }

    uint32_t Module::procTypeCount() const {
      return moduleData().procTypeCount();
    }

    uint32_t Module::regCount() const {
      return moduleData().regCount();
    }

    uint32_t Module::procCount() const {
      return moduleData().procCount();
    }

    void Module::varTypeById(VarTypeId id, VarType &vtype) const {
      moduleData().varTypeById(id, vtype);
    }

    void Module::procTypeById(ProcTypeId id, ProcType &ptype) const {
      moduleData().procTypeById(id, ptype);
    }

    void Module::regById(RegId id, VarSpec &reg) const {
      moduleData().regById(id, reg);
    }

    void Module::procById(ProcId id, Proc &proc) const {
      moduleData().procById(id, proc);
    }

    bool Module::isPacked() const {
      return moduleData().isPacked();
    }

    void Module::pack() {
      moduleData().pack();
    }

    void Module::unpack() {
      moduleData().unpack();
    }

    void Module::drop() {
      moduleData().drop();
    }

    void Module::callProc(ProcId proc, Variable &io) {
      moduleData().callProc(proc, io);
    }

  }
}
