#include "utils.h"
#include "mbuilder.h"
#include "../common/exception.h"

namespace Ant {
  namespace VM {

    using namespace Ant::Common;

    VarTypeId ModuleBuilder::addVarType(int bytes, int count) {
      if(vtypes.size() > MB_UINT_MAX(2))
        throw OutOfRangeException();
      if(bytes < 0 || bytes > MB_UINT_MAX(2))
        throw OutOfRangeException();
      if(count < 0 || count > MB_UINT_MAX(4))
        throw OutOfRangeException();

      MVarType vtype;
      vtype.bytes = uint16_t(bytes);
      vtype.count = uint32_t(count);

      vtypes.push_back(vtype);
      return VarTypeId(vtypes.size() - 1);
    }

    RegId ModuleBuilder::addReg(VarTypeId vtype) {
      if(regs.size() > MB_UINT_MAX(2))
        throw OutOfRangeException();
      if(vtype >= vtypes.size())
        throw OutOfRangeException();

      regs.push_back(vtype);
      return RegId(regs.size() + RESERVED_REGS_COUNT - 1);
    }

    ProcId ModuleBuilder::addProc(int flags, RegId io) {

    }

    int ModuleBuilder::addProcInstr(ProcId id, const Instr &instr) {

    }

    const UUID &ModuleBuilder::createModule() {

    }

  }
}
