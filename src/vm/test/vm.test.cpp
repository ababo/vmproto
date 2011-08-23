#include "vm.test.h"

namespace Ant {
  namespace VM {
    namespace Test {

      bool testVM() {
        bool passed;

        passed = testUtils();
        passed = passed && testModuleBuilder();
        passed = passed && testRuntime();

        return passed;
      }
      
    }
  }
}
