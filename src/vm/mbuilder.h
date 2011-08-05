#ifndef __VM_MBUILDER_INCLUDED__
#define __VM_MBUILDER_INCLUDED__

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

      VarTypeId addVarType(int bytes, int count);
      void addVarTypeVRef(VarTypeId id, VarTypeId vref);
      void addVarTypePRef(VarTypeId id, VarTypeId pref);

      RegId addReg(VarTypeId vtype);
      RegId addReg(VarTypeId vtype, const Variable &constVar);

      ProcId addProc(int flags, RegId io);
      int addProcInstr(ProcId id, const Instr &instr);

      void resetModule();
      const UUID &createModule();

    protected:
      struct MVarType {
        uint32_t count;
        uint16_t bytes;
        std::vector<VarTypeId> vrefs;
        std::vector<VarTypeId> prefs;
      };

      Runtime &rt;

      std::vector<MVarType> vtypes;
      std::vector<VarTypeId> regs;
    };

  }
}

#endif // __VM_MBUILDER_INCLUDED__
