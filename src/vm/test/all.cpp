#include "all.h"

namespace Ant {
  namespace VM {
    namespace Test {

      bool testVM() {
        bool passed;

        passed = testUtils();
        passed = passed && testModuleBuilder();
        passed = passed && testModule();

        return passed;
      }
      
    }
  }
}
