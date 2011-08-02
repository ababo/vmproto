#include "mbuilder.h"

namespace Ant {
  namespace VM {

    const UUID &ModuleBuilder::createModule() {

    }

    VarTypeId ModuleBuilder::addVarType(uint16_t bytes, 
                                        const std::vector<VarTypeId> &vrefs,
                                        const std::vector<ProcTypeId> &prefs,
                                        uint32_t count,
                                        VarTypeId id) {

    }

    ProcTypeId ModuleBuilder::addProcType(const std::vector<VarTypeId> &frame,
                                          ProcTypeId id) {

    }

  }
}
