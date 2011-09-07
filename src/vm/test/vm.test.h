#ifndef __VM_TEST_INCLUDED__
#define __VM_TEST_INCLUDED__

#include "../module.h"

namespace Ant {
  namespace VM {
    namespace Test {

      void createFactorialModule(Module &module);

      bool testUtils();
      bool testModuleBuilder();
      bool testModuleData();

      bool testVM();
    }
  }
}

#endif // __VM_TEST_INCLUDED__
