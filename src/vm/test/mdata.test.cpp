#include "../../exception.h"
#include "../../string.h"
#include "../../test/test.h"
#include "../module.h"
#include "vm.test.h"

namespace {

  using namespace std;
  using namespace Ant;
  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::Runtime::ModuleData";

  bool testFactorial() {
    bool passed = true;
    Module module;

    try {
      SpecifiedVariable<8, 0, 0> io;
      uint64_t &val = *reinterpret_cast<uint64_t*>(io.elts[0].bytes);
      ProcId proc = 0;

      createFactorialModule(module);
      module.unpack();

      val = 0;
      module.callProc(proc, io);
      if(val != 1)
        throw Exception();

      val = 1;
      module.callProc(proc, io);
      if(val != 1)
        throw Exception();

      val = 5;
      module.callProc(proc, io);
      if(val != 120)
        throw Exception();

      val = 10;
      module.callProc(proc, io);
      if(val != 3628800)
        throw Exception();

      val = 20;
      module.callProc(proc, io);
      if(val != 2432902008176640000LLU)
        throw Exception();
    }
    catch(...) { passed = false; }

    IGNORE_THROW(module.drop());

    return printTestResult(subj, "factorial", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testModuleData() {
        bool passed;

        passed = testFactorial();

        return passed;
      }

    }
  }
}
