#include <iostream>
#include <iomanip>

#include "../lang/test/lang.test.h"
#include "../vm/test/vm.test.h"
#include "test.h"

namespace Ant {
  namespace Test {

    using namespace std;

    bool printTestResult(const String subj, const String test,
			 bool testPassed) {
      static const int LINE_WIDTH = 80;      
      int c = LINE_WIDTH - subj.size() - test.size() - 4;
      cout << subj << " (" << test << ")" << setfill('.');
      cout << setw(c) << (testPassed ? "passed" : "FAILED") << endl;

      return testPassed;
    }

    bool testAntOS() {
      bool passed;

      passed = testChar();
      passed = passed && testString();
      passed = passed && Ant::Lang::Test::testLang();
      passed = passed && Ant::VM::Test::testVM();

      return passed;
    }

  }
}
