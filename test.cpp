#include <iostream>

#include "test.h"
#include "lang/test.h"

using namespace std;
using namespace Ant::Lang::Test;

namespace Ant {
  namespace Test {

    bool testAntOS() {
      cout << "AntOS test:" << endl;
      
      testLang();
    }

  }
}
