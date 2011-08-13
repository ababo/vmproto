#ifndef __VM_MBUILDER_INCLUDED__
#define __VM_MBUILDER_INCLUDED__

#include <cstddef>
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

      VarTypeId addVarType(size_t count, size_t bytes);
      void addVarTypeVRef(VarTypeId id, VarTypeId vref);
      void addVarTypePRef(VarTypeId id, VarTypeId pref);

      RegId addReg(VarTypeId vtype);
      RegId addReg(VarTypeId vtype, const Variable &constVar);

      ProcId addProc(unsigned int flags, RegId io);
      size_t addProcInstr(ProcId id, const Instr &instr);

      void resetModule();
      const UUID &createModule();

    protected:
      struct MVarType {
        size_t count;
        size_t bytes;
        std::vector<VarTypeId> vrefs;
        std::vector<VarTypeId> prefs;
      };
      struct MProc {
        unsigned int flags;
        VarTypeId io;
        size_t instrs;
        std::vector<unsigned char> code;
      };

      Runtime &rt;

      std::vector<MVarType> vtypes;
      std::vector<VarTypeId> regs;
      std::vector<MProc> procs;
    };

  }
}

#endif // __VM_MBUILDER_INCLUDED__
