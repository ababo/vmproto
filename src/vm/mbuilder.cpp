#include "../common/exception.h"
#include "../common/sarithm.h"
#include "instr.h"
#include "mbuilder.h"
#include "module.h"
#include "utils.h"

namespace {

  using namespace std;
  using namespace Ant::Common;

  template <class T>
  inline void setFixedArray(const vector<T> &it,
                            FixedArray<T> &target,
                            vector<T> &container) {
    copy(it.begin(), it.end(), back_inserter(container));
    target.set(&container[container.size() - it.size()], it.size());
  }

}

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace Ant::Common;

    VarTypeId ModuleBuilder::assertVarTypeExists(VarTypeId id) const {
      if(id >= vtypes.size())
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

    VarTypeId ModuleBuilder::addVarType(size_t count, size_t bytes) {
      if(vtypes.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(count > MB_UINT_MAX(4))
        throw RangeException();
      if(bytes > MB_UINT_MAX(2))
        throw RangeException();
      if(count ?
         !safeToMultiply(count, bytes) :
         !safeToAdd(bytes, sizeof(size_t)))
        throw RangeException();

      VarType vtype;
      vtype.count = count;
      vtype.bytes = bytes;

      vtypes.push_back(vtype);
      return VarTypeId(vtypes.size() - 1);
    }

    RegId ModuleBuilder::addReg(VarTypeId vtype) {
      regs.push_back(assertVarTypeExists(vtype));
      return RegId(regs.size() - 1);
    }

    ProcId ModuleBuilder::addProc(unsigned int flags, RegId io) {
      if(procs.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(flags >= PFLAG_FIRST_RESERVED)
        throw FlagsException();

      Proc proc;
      proc.flags = flags;
      proc.io = assertRegExists(io);
      procs.push_back(proc);

      ProcCon con;
      con.instrCount = 0;
      con.frames.resize(1);
      con.frames.back().push_back(0);
      procCons.push_back(con);

      return ProcId(procs.size() - 1);
    }

    void ModuleBuilder::applyStackAlloc(ProcId proc) {
      ProcCon &con = procCons[proc];

      for(int i = 0; i < con.frames.size(); i++)
        for(int j = 1; j < con.frames[i].size(); j++)
          if(con.frames[i][j] == con.instrCount + 1)
            throw OperationException();

      con.frames.resize(con.frames.size() + 1);
      con.frames.back().push_back(con.instrCount + 1);
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
               index >= con.frames[i][j] - 1)
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
            throw RangeException();
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
      size_t size = 0;
      for(int i = 0; i < vtypes.size(); i++)
        size += vtypes[i].vrefs.size() + vtypes[i].prefs.size();
      moduleData.refs.reserve(size);

      for(int i = 0; i < vtypes.size(); i++) {
        Runtime::VarTypeData vtypeData;
        const VarType &vtype = vtypes[i];

        vtypeData.count = vtype.count;
        vtypeData.bytes = vtype.bytes;
        setFixedArray(vtype.vrefs, vtypeData.vrefs, moduleData.refs);
        setFixedArray(vtype.prefs, vtypeData.prefs, moduleData.refs);

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
        procData.io = proc.io;
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

      Runtime::ModuleData moduleData;
      fillVarTypes(moduleData);
      moduleData.regs = regs;
      fillProcs(moduleData);

      UUID id = id.generate();
      Runtime::instance().insertModuleData(id, moduleData);
      module.id(id);

      resetModule();
    }

  }
}
