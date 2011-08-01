#include "../../common/string.h"
#include "../../test/all.h"
#include "../mbuilder.h"

namespace Ant {
  namespace VM {
    namespace Test {

      const UUID &buildFactorialModule() {
        ModuleBuilder builder(Runtime::instance());

        return builder.createModule();
      }

    }
  }
}

namespace {

  using namespace Ant::Test;
  using namespace Ant::Common;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::ModuleBuilder";

  bool testFactorial() {
    bool passed;

    try {
      buildFactorialModule();
      // TODO: check the module
      passed = true;
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "factorial", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testModuleBuilder() {
        bool passed;

        passed = testFactorial();

        return passed;
      }

    }
  }
}
