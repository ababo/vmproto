#include "../exception.h"
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

      return iter->second;
    }

    unsigned int Module::varTypeCount() const {
      return moduleData().varTypeCount();
    }

    unsigned int Module::regCount() const {
      return moduleData().regCount();
    }

    unsigned int Module::procCount() const {
      return moduleData().procCount();
    }

    void Module::varTypeById(VarTypeId id, VarType &vtype) const {
      moduleData().varTypeById(id, vtype);
    }

    VarTypeId Module::regTypeById(RegId id) const {
      moduleData().regTypeById(id);
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
