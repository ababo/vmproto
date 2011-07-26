#include "all.h"

namespace Ant {
  namespace Common {
    namespace Test {

      bool testCommon() {        
        bool passed;

        passed = testChar();
        passed = passed && testString();

        return passed;
      }
      
    }
  }
}
