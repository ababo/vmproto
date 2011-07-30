#include "all.h"

namespace Ant {
  namespace VM {
    namespace Test {

      bool testVM() {
        bool passed;

        passed = testRuntime();

        return passed;
      }
      
    }
  }
}
