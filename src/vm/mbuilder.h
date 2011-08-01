#ifndef __VM_MBUILDER_INCLUDED__
#define __VM_MBUILDER_INCLUDED__

#include <stdint.h>
#include <vector>

#include "uuid.h"
#include "module.h"
#include "runtime.h"
#include "instr.h"

namespace Ant {
  namespace VM {

    class ModuleBuilder {
    public:
      ModuleBuilder(Runtime &rt) : rt(rt) {}

      VarTypeId addVarType(VarTypeId id);
      VarTypeId addVarType(uint16_t bytes, 
                           const std::vector<VarTypeId> &vrefs,
                           const std::vector<ProcTypeId> &prefs,
                           uint32_t count,
                           VarTypeId id = 0);
      ProcTypeId addProcType(ProcTypeId id);
      ProcTypeId addProcType(const std::vector<VarTypeId> &frame,
                             ProcTypeId id = 0);
      ProcId addProc(ProcId id);
      ProcId addProc(uint32_t flags,
                     ProcTypeId ptypeId,
                     const std::vector<Instr> &code,
                     ProcId id = 0);

      void resetModule();
      const UUID &createModule();

    protected:
      Runtime &rt;
    };

  }
}

#endif // __VM_MBUILDER_INCLUDED__
