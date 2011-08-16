#ifndef __VM_MODULE_INCLUDED__
#define __VM_MODULE_INCLUDED__

#include <cstddef>
#include <vector>

#include "../common/uuid.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    struct VarType {
      size_t count;
      size_t bytes;
      std::vector<VarTypeId> vrefs;
      std::vector<VarTypeId> prefs;
    };

    struct Proc {
      unsigned int flags;
      VarTypeId io;
      std::vector<VMCodeByte> code;
    };

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FUNCTION = 0x2,
      PFLAG_FIRST_RESERVED = 0x4
    };

    const unsigned int RESERVED_REGS_COUNT = 8;

    class Module {
    public:
      Module() {}
      Module(const Common::UUID &id) : _id(id) {}

      const Common::UUID &id() const { return _id; }
      void id(const Common::UUID &id) { _id = id; }

      unsigned int varTypeCount() const;
      unsigned int regCount() const;
      unsigned int procCount() const;

      void varTypeById(VarTypeId id, VarType &vtype) const;
      VarTypeId regTypeById(RegId id) const;
      void procById(ProcId id, Proc &proc) const;

      bool isExistent() const;
      bool isPacked() const;

      void pack();
      void unpack();
      void drop();

      void callFunc(ProcId func, Variable &io) const;
      void callProc(ProcId proc, Variable &io);

    protected:
      const Runtime::ModuleData &moduleData() const;

      Common::UUID _id;
    };

  }
}

#endif // __VM_MODULE_INCLUDED__
