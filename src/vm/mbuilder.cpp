#include "utils.h"
#include "mbuilder.h"
#include "../common/exception.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace Ant::Common;

    VarTypeId ModuleBuilder::addVarType(int bytes, int count) {
      if(vtypes.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(bytes < 0 || bytes > MB_UINT_MAX(4))
        throw RangeException();
      if(count < 0 || count > MB_UINT_MAX(4))
        throw RangeException();

      MVarType vtype;
      vtype.bytes = uint32_t(bytes);
      vtype.count = uint32_t(count);

      vtypes.push_back(vtype);
      return VarTypeId(vtypes.size() - 1);
    }

    RegId ModuleBuilder::addReg(VarTypeId vtype) {
      if(regs.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(vtype >= vtypes.size())
        throw NotFoundException();

      regs.push_back(vtype);
      return RegId(regs.size() + RESERVED_REGS_COUNT - 1);
    }

    ProcId ModuleBuilder::addProc(int flags, RegId io) {
      if(procs.size() >= MB_UINT_MAX(2))
        throw RangeException();
      if(flags < 0 || flags >= PFLAG_FIRST_RESERVED)
        throw FlagsException();
      if(io < RESERVED_REGS_COUNT)
        throw ArgumentException();
      if(io >= regs.size() + RESERVED_REGS_COUNT)
        throw NotFoundException();

      MProc proc;
      proc.flags = uint16_t(flags);
      proc.io = uint16_t(io);
      proc.instrs = 0;

      procs.push_back(proc);
      return ProcId(procs.size() - 1);
    }

    int ModuleBuilder::addProcInstr(ProcId id, const Instr &instr) {
      if(id >= procs.size())
        throw NotFoundException();
      
      MProc &proc = procs[id];
      if(proc.instrs >= MB_UINT_MAX(4))
        throw RangeException();

      size_t size = instr.size();
      proc.code.resize(proc.code.size() + size);
      copy(instr.data(), instr.data() + size, back_inserter(proc.code));
      return ++proc.instrs;
    }

    const UUID &ModuleBuilder::createModule() {

    }

  }
}
