#include "../../common/string.h"
#include "../../test/all.h"
#include "../runtime.h"

namespace {

  using namespace Ant::Test;
  using namespace Ant::Common;

  const String subj = "Ant::VM::Runtime";

  bool testFactorial() {
    bool passed;

    passed = false;

    return printTestResult(subj, "factorial", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testRuntime() {
        bool passed;

        passed = testFactorial();

        return passed;
      }

    }
  }
}
