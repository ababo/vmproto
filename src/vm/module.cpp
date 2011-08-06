#include "module.h"

namespace Ant {
  namespace VM {

    unsigned int Module::varTypeCount() const {

      return 0;
    }

    unsigned int Module::regCount() const {

      return 0;
    }

    unsigned int Module::procCount() const {

      return 0;
    }

    void Module::varTypeById(VarTypeId id, VarType &vtype) const {

    }

    VarTypeId Module::regTypeById(RegId id) const {

      return 0;
    }

    void Module::procById(ProcId id, Proc &proc) const {

    }


    void Module::unpack() {

    }

    void Module::drop() {

    }

    void Module::callFunc(ProcId func, Variable &io) const {


    }

  }
}
