#ifndef __TEST_ALL_INCLUDED__
#define __TEST_ALL_INCLUDED__

#include "../common/string.h"

namespace Ant {
  namespace Test {

    bool printTestResult(const Common::String subj,
                         const Common::String test,
                         bool testPassed,
                         int lineWidth = 80);

    bool testAntOS();

  }
}

#endif // __TEST_ALL_INCLUDED__
