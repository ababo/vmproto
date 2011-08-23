#ifndef __VM_MODULE_INCLUDED__
#define __VM_MODULE_INCLUDED__

#include <cstddef>
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
