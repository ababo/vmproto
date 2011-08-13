#include "../../common/string.h"
#include "../../test/all.h"
#include "../mbuilder.h"
#include "../module.h"

namespace Ant {
  namespace VM {
    namespace Test {

      const UUID &createFactorialModule(Runtime &rt) {
        ModuleBuilder builder(rt);

        VarTypeId wordType = builder.addVarType(1, 8);

        RegId io = builder.addReg(wordType);
        RegId pr = builder.addReg(wordType);

        ProcId proc = builder.addProc(PFLAG_EXTERNAL | PFLAG_FUNCTION, io);
        builder.addProcInstr(proc, ASTInstr(pr));
        builder.addProcInstr(proc, MOVM8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr));
        builder.addProcInstr(proc, DECInstr(io));
        builder.addProcInstr(proc, JNZInstr(-2));
        builder.addProcInstr(proc, MOVN8Instr(pr, io));
        builder.addProcInstr(proc, FSTInstr());
        builder.addProcInstr(proc, RETInstr());

        return builder.createModule();
      }

    }
  }
}

namespace {

  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::Common;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::ModuleBuilder";

  bool testFactorial() {
    Runtime &rt = Runtime::instance();
    UUID module;

    bool passed;

    try {
      module = createFactorialModule(rt);
      // TODO: check the module properties
      passed = true;
    }
    catch(...) { passed = false; }

    try { rt.deleteModule(module); }
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
