#ifndef __VM_TEST_ALL_INCLUDED__
#define __VM_TEST_ALL_INCLUDED__

#include "../runtime.h"

namespace Ant {
  namespace VM {
    namespace Test {

      const UUID &createFactorialModule(Runtime &rt);

      bool testModuleBuilder();
      bool testRuntime();

      bool testVM();
    }
  }
}

#endif // __VM_TEST_ALL_INCLUDED__
