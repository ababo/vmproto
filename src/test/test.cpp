#include <iostream>
#include <iomanip>

#include "../lang/test/lang.test.h"
#include "../vm/test/vm.test.h"
#include "test.h"

namespace Ant {
  namespace Test {

    bool printTestResult(const String subj,
                         const String test,
                         bool testPassed) {
      using namespace std;

      static const int LINE_WIDTH = 80;      
      int c = LINE_WIDTH - subj.length() - test.length() - 4;
      wcout << subj << L" (" << test << L")" << setfill(L'.');
      wcout << setw(c) << (testPassed ? L"passed" : L"FAILED") << endl;
      
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
