#include "vm.test.h"

namespace Ant {
  namespace VM {
    namespace Test {

      bool testVM() {
        bool passed;

        passed = testUtil();
        passed = passed && testModuleBuilder();
        passed = passed && testModuleData();

        return passed;
      }
      
    }
  }
}
