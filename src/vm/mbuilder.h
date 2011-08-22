#ifndef __VM_MBUILDER_INCLUDED__
#define __VM_MBUILDER_INCLUDED__

#include <cstddef>
#include <vector>

#include "runtime.h"

namespace Ant {
  namespace VM {

    class Instr;
    class Module;

    class ModuleBuilder {
      friend class Instr;
    public:
      unsigned int varTypeCount() const { return vtypes.size(); }
      unsigned int regCount() const { return regs.size(); }
      unsigned int procCount() const { return procs.size(); }

      void varTypeById(VarTypeId id, VarType &vtype) const {
        vtype = vtypes[assertVarTypeExists(id)];
      }
      VarTypeId regTypeById(RegId id) const {
        return regs[assertRegExists(id)];
      }
      void procById(ProcId id, Proc &proc) const {
        proc = procs[assertProcExists(id)];
      }

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
      struct ProcCon {
        size_t instrCount;
        // ...
      };

      VarTypeId assertVarTypeExists(VarTypeId id) const;
      RegId assertRegExists(RegId id) const;
      ProcId assertProcExists(ProcId id) const;

      void fillVarTypes(Runtime::ModuleData &moduleData) const;
      void fillProcs(Runtime::ModuleData &moduleData) const;

      void applyStackAlloc(ProcId proc);
      void applyStackFree(ProcId proc);
      void applyInstrOffset(ProcId proc, int offset);

      bool moduleConsistent() const;

      std::vector<VarType> vtypes;
      std::vector<VarTypeId> regs;
      std::vector<Proc> procs;
      std::vector<ProcCon> procCons;
    };

  }
}

#endif // __VM_MBUILDER_INCLUDED__
