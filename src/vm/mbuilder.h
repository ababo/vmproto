#ifndef __VM_MBUILDER_INCLUDED__
#define __VM_MBUILDER_INCLUDED__

#include <stdint.h>

#include "uuid.h"
#include "runtime.h"

namespace Ant {
  namespace VM {

    class ModuleBuilder {
    public:
      ModuleBuilder(Runtime &rt) : rt(rt) {}

      const UUID &finalizeModule();

    protected:
      Runtime &rt;
    };

  }
}

#endif // __VM_MBUILDER_INCLUDED__
