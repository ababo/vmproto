#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <stdint.h>
#include <vector>

#include "uuid.h"
#include "module.h"
#include "../common/singleton.h"

namespace Ant {
  namespace VM {

    struct Variable {

    };

    template <int Bytes>
    struct StaticVariable : public Variable {
      uint8_t bytes[Bytes];
    };

    class Runtime : public Common::Singleton<Runtime> {
      friend class Common::Singleton<Runtime>;
    public:
      void unpackModule(const UUID &moduleId);

      void callProcedure(const UUID &moduleId,
                         uint16_t moduleProc,
                         std::vector<Variable*> &ioframe);

    private:
      Runtime() : Common::Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
