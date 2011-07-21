#include "all.h"

namespace Ant {
  namespace Common {
    namespace Test {

      bool testCommon() {        
        return
          testChar() &&
          testString();
      }
      
    }
  }
}
