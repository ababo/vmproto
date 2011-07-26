#include "all.h"

namespace Ant {
  namespace Lang {
    namespace Test {

      bool testLang() {
        bool passed;

        passed = testLexer();

        return passed;
      }
      
    }
  }
}
