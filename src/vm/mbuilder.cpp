#include "../exception.h"
#include "instr.h"
#include "mbuilder.h"
#include "mdata.h"
#include "module.h"

namespace {

  using namespace std;
  using namespace Ant;

  template<class T> inline void setFixedArray(const vector<T> &it,
                                              FixedArray<T> &target,
                                              vector<T> &container) {
    copy(it.begin(), it.end(), back_inserter(container));
    target.set(&container[container.size() - it.size()], it.size());
  }

}

namespace Ant {
  namespace VM {

    using namespace std;

    VarTypeId ModuleBuilder::assertVarTypeExists(VarTypeId id) const {
      if(id >= vtypes.size())
        throw NotFoundException();
      return id;
    }

    ProcTypeId ModuleBuilder::assertProcTypeExists(ProcTypeId id) const {
      if(id >= ptypes.size())
        throw NotFoundException();
      return id;
    }

    RegId ModuleBuilder::assertRegExists(RegId id) const {
      if(id >= regs.size())
        throw NotFoundException();
      return id;
    }

    ProcId ModuleBuilder::assertProcExists(ProcId id) const {
      if(id >= procs.size())
        throw NotFoundException();
      return id;
    }

    RegId ModuleBuilder::assertRegAllocated(ProcId proc, RegKind kind,
					    RegId reg) const {
      if(regs[reg].flags & VFLAG_PERSISTENT)
        return reg;

      const ProcCon &con = procCons[proc];
      for(int i = con.frames.size() - 1; i >= 0; i--)
        if(con.frames[i].reg == reg)
	  if((kind == RK_NOVOID && con.frames[i].kind != RK_VOID) ||
	     kind == con.frames[i].kind)
	    return reg;
	  else break;

      throw OperationException();
    }

    ProcId ModuleBuilder::assertProcCallable(ProcId proc,
                                             ProcId targetProc) const {
      const Frame &frame = procCons[proc].frames.back();

      if(frame.kind != RK_NOREF)
        throw OperationException();

      const VarSpec &svspec = regs[frame.reg];
      const VarSpec &tvspec = regs[ptypes[procs[targetProc].ptype].io];

      if(svspec.flags & VFLAG_NON_FIXED ||
         svspec.vtype != tvspec.vtype ||
         svspec.count != tvspec.count)
        throw OperationException();

      return targetProc;
    }

    VarTypeId ModuleBuilder::addVarType(uint32_t bytes) {
      if(vtypes.size() >= MODULE_VTYPES_MAX)
        throw RangeException();
      if(bytes > ELT_BYTES_MAX)
        throw RangeException();

      VarType vtype;
      vtype.bytes = bytes;

      vtypes.push_back(vtype);
      return VarTypeId(vtypes.size() - 1);
    }

    void ModuleBuilder::addVarTypeVRef(VarTypeId id, uint32_t flags,
                                       VarTypeId vtype, size_t count) {
     if(flags & ~VFLAG_NON_FIXED)
       throw FlagsException();
      if(!count || count > VAR_COUNT_MAX)
        throw RangeException();

      VarType &vt = vtypes[assertVarTypeExists(id)];
      if(vt.vrefs.size() >= ELT_VREFS_MAX)
        throw RangeException();

      VarSpec vspec;
      vspec.flags = flags;
      vspec.vtype = assertVarTypeExists(vtype);
      vspec.count = count;

      vt.vrefs.push_back(vspec);
    }

    ProcTypeId ModuleBuilder::addProcType(uint32_t flags, RegId io) {
      if(ptypes.size() >= MODULE_PTYPES_MAX)
        throw RangeException();
      if(flags >= PTFLAG_FIRST_RESERVED)
        throw FlagsException();
      if(regs[assertRegExists(io)].flags & VFLAG_NON_FIXED)
        throw TypeException();

      ProcType ptype;
      ptype.flags = flags;
      ptype.io = io;

      ptypes.push_back(ptype);
      return ProcTypeId(ptypes.size() - 1);
    }

    RegId ModuleBuilder::addReg(uint32_t flags, VarTypeId vtype,
				size_t count) {
      if(regs.size() >= MODULE_REGS_MAX)
        throw RangeException();
      if(flags >= VFLAG_FIRST_RESERVED)
        throw FlagsException();
      if(!count || count > VAR_COUNT_MAX)
        throw RangeException();

      VarSpec reg;
      reg.flags = flags;
      reg.vtype = assertVarTypeExists(vtype);
      reg.count = count;

      regs.push_back(reg);
      return RegId(regs.size() - 1);
    }

    ProcId ModuleBuilder::addProc(uint32_t flags, ProcTypeId ptype) {
      if(procs.size() >= MODULE_PROCS_MAX)
        throw RangeException();
      if(flags >= PFLAG_FIRST_RESERVED)
        throw FlagsException();

      Proc proc;
      proc.flags = flags;
      proc.ptype = assertProcTypeExists(ptype);
      procs.push_back(proc);

      ProcCon con;
      con.instrCount = 0;
      con.frames.resize(1);
      con.frames.back().firstInstr = 0;
      con.frames.back().kind = RK_NOREF;
      con.frames.back().reg = ptypes[ptype].io;
      procCons.push_back(con);

      return ProcId(procs.size() - 1);
    }

    void ModuleBuilder::applyBeginFrame(ProcId proc, RegKind kind, RegId reg) {
      ProcCon &con = procCons[proc];

      for(int i = 0; i < con.frames.size(); i++)
        for(int j = 0; j < con.frames[i].jumps.size(); j++)
          if(con.frames[i].jumps[j] == con.instrCount + 1)
            throw OperationException();

      con.frames.resize(con.frames.size() + 1);
      con.frames.back().firstInstr = con.instrCount + 1;
      con.frames.back().kind = kind;
      con.frames.back().reg = reg;
    }

    void ModuleBuilder::applyBeginFrame(ProcId proc, ptrdiff_t offset) {
      applyBeginFrame(proc, RK_VOID, 0);      
      try { applyInstrOffset(proc, offset); }
      catch(...) { procCons[proc].frames.pop_back(); throw; }
    }

    void ModuleBuilder::applyEndFrame(ProcId proc) {
      ProcCon &con = procCons[proc];

      if(con.frames.size() == 1)
        throw OperationException();

      Frame &frame = con.frames.back();
      for(int i = 0; i < frame.jumps.size(); i++)
        if(frame.jumps[i] > con.instrCount)
          throw OperationException();

      con.frames.pop_back();
    }

    void ModuleBuilder::applyInstrOffset(ProcId proc, ptrdiff_t offset) {
      if(offset < INSTR_OFFSET_MIN || offset > INSTR_OFFSET_MAX)
        throw RangeException();

      ptrdiff_t index = ptrdiff_t(procCons[proc].instrCount) + offset;

      if(index < 0)
        throw RangeException();

      applyInstrIndex(proc, size_t(index));
    }

    void ModuleBuilder::applyInstrIndex(ProcId proc, size_t index) {
      applyDefault(proc);

      ProcCon &con = procCons[proc];
      Frame &frame = con.frames.back();

      if(index > con.instrCount) {
        if(index >= PROC_INSTR_MAX)
          throw RangeException();

        for(int i = 0; i < con.frames.size() - 1; i++)
          for(int j = 0; j < con.frames[i].jumps.size(); j++)
            if(con.frames[i].jumps[j] >= frame.firstInstr &&
               index >= con.frames[i].jumps[j])
              throw RangeException();
      }
      else if(index < frame.firstInstr)
        throw RangeException();

      frame.jumps.push_back(index);
    }

    void ModuleBuilder::applyDefault(ProcId proc) {
      ProcCon &con = procCons[proc];
      Frame &frame = con.frames.back();

      for(int i = 0; i < con.frames.size() - 1; i++)
        for(int j = 0; j < con.frames[i].jumps.size(); j++)
          if(con.frames[i].jumps[j] >= con.frames[i].firstInstr &&
             con.instrCount == con.frames[i].jumps[j] - 1)
            throw OperationException();
    }

    size_t ModuleBuilder::addProcInstr(ProcId id, const Instr &instr) {
      if(procCons[assertProcExists(id)].instrCount >= PROC_INSTR_MAX)
        throw RangeException();
      instr.assertConsistency(*this, id);

      copy(instr.data(), instr.data() + instr.size(),
           back_inserter(procs[id].code));

      return procCons[id].instrCount++;
    }

    void ModuleBuilder::assertConsistency() const {
      for(VarTypeId vtype = RESERVED_VAR_TYPE_COUNT; vtype < vtypes.size();
          vtype++)
        if(!vtypes[vtype].bytes && !vtypes[vtype].vrefs.size() &&
           !vtypes[vtype].prefs.size())
          throw OperationException();

      for(ProcId proc = 0; proc < procs.size(); proc++) {
        const ProcCon &con = procCons[proc];

        if(con.frames.size() != 1)
          throw OperationException();

        const Frame &frame = con.frames.back();
        for(int i = 0; i < frame.jumps.size(); i++)
          if(frame.jumps[i] >= con.instrCount)
            throw OperationException();
      }
    }

    void ModuleBuilder::fillVarTypes(Runtime::ModuleData &moduleData) const {
      size_t vsize = 0, psize = 0;
      for(int i = 0; i < vtypes.size(); i++) {
        vsize += vtypes[i].vrefs.size();
	psize += vtypes[i].prefs.size();
      }
      moduleData.vrefs.reserve(vsize);
      moduleData.prefs.reserve(psize);

      for(int i = 0; i < vtypes.size(); i++) {
        Runtime::VarTypeData vtypeData;
        const VarType &vtype = vtypes[i];

        vtypeData.bytes = vtype.bytes;
        setFixedArray(vtype.vrefs, vtypeData.vrefs, moduleData.vrefs);
        setFixedArray(vtype.prefs, vtypeData.prefs, moduleData.prefs);

        moduleData.vtypes.push_back(vtypeData);
      }
    }

    void ModuleBuilder::fillProcs(Runtime::ModuleData &moduleData) const {
      size_t size = 0;
      for(int i = 0; i < procs.size(); i++)
        size += procs[i].code.size();
      moduleData.code.reserve(size);

      for(int i = 0; i < procs.size(); i++) {
        Runtime::ProcData procData;
        const Proc &proc = procs[i];

        procData.flags = proc.flags;
        procData.ptype = proc.ptype;
        setFixedArray(proc.code, procData.code, moduleData.code);

        moduleData.procs.push_back(procData);
      }
    }

    void ModuleBuilder::resetModule() {
      vtypes.clear();
      regs.clear();
      procs.clear();
      procCons.clear();

      // exception descriptor
      addReg(VFLAG_PERSISTENT | VFLAG_THREAD_LOCAL, addVarType(8));
    }

    void ModuleBuilder::createModule(Module &module) {
      assertConsistency();

      UUID id = id.generate();
      Runtime::ModuleData moduleData(id);
      fillVarTypes(moduleData);
      moduleData.ptypes = ptypes;
      moduleData.regs = regs;
      fillProcs(moduleData);

      Runtime::instance().insertModuleData(id, moduleData);
      module.id(id);

      resetModule();
    }

  }
}
