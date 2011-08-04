#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include "uuid.h"
#include "module.h"
#include "../common/singleton.h"

namespace Ant {
  namespace VM {

    struct Variable {};

    template <int ByteCount, int VRefCount, int PRefCount, int EltCount>
    struct StaticVariable : public Variable {
      struct {
        uint8_t bytes[ByteCount];
        Variable *vrefs[VRefCount];
        NativeCode prefs[PRefCount];
      } elts[EltCount];
    };

    class Runtime : public Common::Singleton<Runtime> {
      friend class Common::Singleton<Runtime>;
    public:
      void unpackModule(const UUID &id);
      void deleteModule(const UUID &id);

      void callProcedure(const UUID &module, ProcId proc, Variable &io);

    private:
      Runtime() : Common::Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
