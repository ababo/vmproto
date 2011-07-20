#include <iostream>
#include <iomanip>

#include "all.h"
#include "../common/test/all.h"
#include "../lang/test/all.h"

namespace Ant {
  namespace Test {

    bool printTestResult(const Common::String subj,
                         const Common::String test,
                         bool testPassed) {
      using namespace std;

      static const int LINE_WIDTH = 80;      
      int c = LINE_WIDTH - subj.length() - test.length() - 4;
      wcout << subj << L" (" << test << L")" << setfill(L'.');
      wcout << setw(c) << (testPassed ? L"passed" : L"FAILED") << endl;
      
      return testPassed;
    }

    bool testAntOS() {
      return
        Ant::Common::Test::testCommon() &&
        Ant::Lang::Test::testLang();
    }

  }
}
