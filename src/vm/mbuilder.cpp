#include "../exception.h"
#include "../sarithm.h"
#include "instr.h"
#include "mbuilder.h"
#include "mdata.h"
#include "module.h"
#include "utils.h"

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

    RegId ModuleBuilder::assertRegAllocated(ProcId proc, RegId reg) const {
      const ProcCon &con = procCons[proc];
      for(int i = con.allocs.size() - 1; i >= 0; i--)
        if(con.allocs[i] == reg)
          return reg;

      throw OperationException();
    }

    size_t ModuleBuilder::assertCountInRange(VarTypeId vtype, size_t count) {
      assertVarTypeExists(vtype);

      if(count > MB_UINT_MAX(4))
        throw RangeException();

      if(count ?
         !safeToMultiply(count, vtypes[vtype].bytes + sizeof(size_t)) :
         !safeToAdd(vtypes[vtype].bytes, 2 * sizeof(size_t)))
        throw RangeException();

      return count;
    }

    VarTypeId ModuleBuilder::addVarType(size_t bytes) {
      if(vtypes.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(bytes > MB_UINT_MAX(2))
        throw RangeException();

      VarType vtype;
      vtype.bytes = bytes;

      vtypes.push_back(vtype);
      return VarTypeId(vtypes.size() - 1);
    }

    ProcTypeId ModuleBuilder::addProcType(uint32_t flags, RegId io) {
      if(ptypes.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(flags >= PTFLAG_FIRST_RESERVED)
        throw FlagsException();

      ProcType ptype;
      ptype.flags = flags;
      ptype.io = assertRegExists(io);

      ptypes.push_back(ptype);
      return ProcTypeId(ptypes.size() - 1);
    }

    RegId ModuleBuilder::addReg(VarTypeId vtype, size_t count) {
      VarSpec reg;
      reg.vtype = vtype;
      reg.count = assertCountInRange(vtype, count);

      regs.push_back(reg);
      return RegId(regs.size() - 1);
    }

    ProcId ModuleBuilder::addProc(uint32_t flags, ProcTypeId ptype) {
      if(procs.size() >= MB_UINT_MAX(2))
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
      con.frames.back().push_back(0);
      con.allocs.push_back(ptypes[ptype].io);
      procCons.push_back(con);

      return ProcId(procs.size() - 1);
    }

    void ModuleBuilder::applyStackAlloc(ProcId proc, RegId reg) {
      ProcCon &con = procCons[proc];

      for(int i = 0; i < con.frames.size(); i++)
        for(int j = 1; j < con.frames[i].size(); j++)
          if(con.frames[i][j] == con.instrCount + 1)
            throw OperationException();

      con.frames.resize(con.frames.size() + 1);
      con.frames.back().push_back(con.instrCount + 1);
      con.allocs.push_back(reg);
    }

    void ModuleBuilder::applyStackFree(ProcId proc) {
      ProcCon &con = procCons[proc];

      if(con.frames.size() == 1)
        throw OperationException();

      ProcCon::Frame &frame = con.frames.back();
      for(int i = 1; i < frame.size(); i++)
        if(frame[i] > con.instrCount)
          throw OperationException();

      con.frames.pop_back();
      con.allocs.pop_back();
    }

    void ModuleBuilder::applyInstrOffset(ProcId proc, int offset) {
      if(offset < MB_INT_MIN(1) || offset > MB_INT_MAX(1))
        throw RangeException();

      ptrdiff_t index = ptrdiff_t(procCons[proc].instrCount) + offset;

      if(index < 0)
        throw RangeException();

      applyInstrIndex(proc, size_t(index));
    }

    void ModuleBuilder::applyInstrIndex(ProcId proc, size_t index) {
      applyDefault(proc);

      ProcCon &con = procCons[proc];
      ProcCon::Frame &frame = con.frames.back();

      if(index > con.instrCount) {
        if(index >= MB_UINT_MAX(4))
          throw RangeException();

        for(int i = 0; i < con.frames.size() - 1; i++)
          for(int j = 1; j < con.frames[i].size(); j++)
            if(con.frames[i][j] >= frame[0] &&
               index >= con.frames[i][j])
              throw RangeException();
      }
      else if(index < frame[0])
        throw RangeException();

      frame.push_back(index);
    }

    void ModuleBuilder::applyDefault(ProcId proc) {
      ProcCon &con = procCons[proc];
      ProcCon::Frame &frame = con.frames.back();

      for(int i = 0; i < con.frames.size() - 1; i++)
        for(int j = 1; j < con.frames[i].size(); j++)
          if(con.frames[i][j] >= frame[0] &&
             con.instrCount == con.frames[i][j] - 1)
            throw OperationException();
    }

    size_t ModuleBuilder::addProcInstr(ProcId id, const Instr &instr) {
      if(procCons[assertProcExists(id)].instrCount >= MB_UINT_MAX(4))
        throw RangeException();
      instr.assertConsistency(*this, id);

      copy(instr.data(), instr.data() + instr.size(),
           back_inserter(procs[id].code));

      return procCons[id].instrCount++;
    }

    void ModuleBuilder::assertConsistency() const {
      for(ProcId proc = 0; proc < procs.size(); proc++) {
        const ProcCon &con = procCons[proc];

        if(con.frames.size() != 1)
          throw OperationException();

        const ProcCon::Frame &frame = con.frames.back();
        for(int i = 1; i < frame.size(); i++)
          if(frame[i] >= con.instrCount)
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
