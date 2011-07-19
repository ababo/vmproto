#include <iostream>
#include <iomanip>

#include "all.h"
#include "../common/test/all.h"
#include "../lang/test/all.h"

namespace Ant {
  namespace Test {

    bool printTestResult(const Common::String subj,
                         const Common::String test,
                         bool testPassed,
                         int lineWidth) {
      using namespace std;

      int count = lineWidth - subj.length() - test.length() - 4;
      cout << subj << " (" << test << ")" << setfill('.');
      cout << setw(count) << (testPassed ? "passed" : "FAILED") << endl;
      return testPassed;
    }

    bool testAntOS() {
      return
        Ant::Common::Test::testCommon() &&
        Ant::Lang::Test::testLang();
    }

  }
}
