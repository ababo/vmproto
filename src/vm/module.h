#ifndef __VM_MODULE_INCLUDED__
#define __VM_MODULE_INCLUDED__

#include <cstddef>
#include <stdint.h>
#include <vector>

#include "../uuid.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    class Module {
    public:
      Module() { init(UUID()); }
      Module(const UUID &id) { init(id); }
      ~Module() { id(UUID()); }

      const UUID &id() const { return _id; }
      void id(const UUID &id);

      uint32_t varTypeCount() const;
      uint32_t procTypeCount() const;
      uint32_t regCount() const;
      uint32_t procCount() const;

      void varTypeById(VarTypeId id, VarType &vtype) const;
      void procTypeById(ProcTypeId id, ProcType &ptype) const;
      void regById(RegId id, VarSpec &reg) const;
      void procById(ProcId id, Proc &proc) const;

      bool isExistent() const;
      bool isPacked() const;

      void pack();
      void unpack();
      void drop();

      void callProc(ProcId proc, Variable &io);

    protected:
      void init(const UUID &id) {
        _id = id, iter = Runtime::instance().modules.end();
      }

      Runtime::ModuleData &moduleData() const;

      UUID _id;
      mutable Runtime::ModuleDataIterator iter;
    };

  }
}

#endif // __VM_MODULE_INCLUDED__
