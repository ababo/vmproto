#include <iostream>

#include "all.h"
#include "../common/test/all.h"
#include "../lang/test/all.h"

using namespace std;
using namespace Ant::Common::Test;
using namespace Ant::Lang::Test;

namespace Ant {
  namespace Test {

    bool testAntOS() {
      cout << "AntOS test:" << endl;
      
      testCommon();
      testLang();
    }

  }
}
