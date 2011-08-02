#include "../../common/string.h"
#include "../../test/all.h"
#include "../mbuilder.h"
#include "../module.h"

namespace Ant {
  namespace VM {
    namespace Test {

      using namespace std;

      const UUID &buildFactorialModule() {
        ModuleBuilder builder(Runtime::instance());

        vector<VarTypeId> vtypes;
        vector<ProcTypeId> ptypes;
        VarTypeId wordTypeId = builder.addVarType(8, vtypes, ptypes, 1);

        vtypes.push_back(wordTypeId);
        ProcTypeId factTypeId = builder.addProcType(vtypes);

        vector<Instr> code;
        code.push_back(MOVM8Instr(1, FIRST_LOCAL_REG));
        code.push_back(MULInstr(FIRST_LOCAL_REG,
                                FIRST_CALL_REG,
                                FIRST_LOCAL_REG));
        code.push_back(DECInstr(FIRST_CALL_REG));
        code.push_back(JNZInstr(-2));
        code.push_back(MOVN8Instr(FIRST_LOCAL_REG,
                                  FIRST_CALL_REG));
        code.push_back(RETInstr());

        

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
