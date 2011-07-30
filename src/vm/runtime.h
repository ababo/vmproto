#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <stdint.h>
#include <vector>

#include "uuid.h"
#include "module.h"

namespace Ant {
  namespace VM {

    struct Variable {

    };

    template <int Bytes>
    struct StaticVariable : public Variable {
      uint8_t bytes[Bytes];
    };

    class Runtime {
    public:
      const UUID &addModule(Module *module);

      void callProcedure(const UUID &moduleId,
                         uint16_t moduleProc,
                         std::vector<Variable&> &frame);
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
