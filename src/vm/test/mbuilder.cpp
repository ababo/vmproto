#include "../../common/string.h"
#include "../../test/all.h"
#include "../mbuilder.h"
#include "../module.h"

namespace Ant {
  namespace VM {
    namespace Test {

      const UUID &createFactorialModule() {
        ModuleBuilder builder(Runtime::instance());

        VarTypeId wordType = builder.addVarType(8, 1);

        FrameId ioframe = builder.addFrame();
        RegId ioreg = builder.addFrameReg(ioframe, wordType);
        FrameId lframe = builder.addFrame();
        RegId lreg = builder.addFrameReg(lframe, wordType);

        ProcId proc = builder.addProc(PFLAG_EXTERNAL, ioreg);
        builder.addProcInstr(proc, AFRMInstr(lframe));
        builder.addProcInstr(proc, MOVM8Instr(1, lreg));
        builder.addProcInstr(proc, MULInstr(ioreg, lreg, lreg));
        builder.addProcInstr(proc, DECInstr(ioreg));
        builder.addProcInstr(proc, JNZInstr(-2));
        builder.addProcInstr(proc, MOVN8Instr(lreg, ioreg));
        builder.addProcInstr(proc, FFRMInstr());
        builder.addProcInstr(proc, RETInstr());

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
      createFactorialModule();
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
