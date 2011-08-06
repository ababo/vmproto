#ifndef __VM_MBUILDER_INCLUDED__
#define __VM_MBUILDER_INCLUDED__

#include <cstddef>
#include <vector>

#include "module.h"
#include "instr.h"

namespace Ant {
  namespace VM {

    class ModuleBuilder {
    public:
      VarTypeId addVarType(size_t count, size_t bytes);
      void addVarTypeVRef(VarTypeId id, VarTypeId vref);
      void addVarTypePRef(VarTypeId id, VarTypeId pref);

      RegId addReg(VarTypeId vtype);
      RegId addReg(VarTypeId vtype, const Variable &constVar);

      ProcId addProc(unsigned int flags, RegId io);
      size_t addProcInstr(ProcId id, const Instr &instr);

      void resetModule();
      void createModule(Module &module);

    protected:
      std::vector<VarType> vtypes;
      std::vector<VarTypeId> regs;
      std::vector<Proc> procs;
      std::vector<unsigned int> instrs;
    };

  }
}

#endif // __VM_MBUILDER_INCLUDED__
