#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include "uuid.h"
#include "module.h"
#include "../common/singleton.h"

namespace Ant {
  namespace VM {

    static const unsigned int RESERVED_REGS_COUNT = 8;

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
