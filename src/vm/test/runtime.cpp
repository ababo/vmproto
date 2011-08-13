#include "../../common/exception.h"
#include "../../common/string.h"
#include "../../test/all.h"
#include "../runtime.h"
#include "all.h"

namespace {

  using namespace std;
  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::Common;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::Runtime";

  bool testFactorial() {
    Runtime &rt = Runtime::instance();
    UUID module;

    StaticVariable<1, 8, 0, 0> io;
    uint64_t &val = *reinterpret_cast<uint64_t*>(io.elts[0].bytes);
    ProcId proc = 0;

    bool passed = true;

    try {
      module = createFactorialModule(rt);
      rt.unpackModule(module);

      val = 0;
      rt.callProcedure(module, proc, io);
      if(val != 1)
        throw Exception();

      val = 1;
      rt.callProcedure(module, proc, io);
      if(val != 1)
        throw Exception();

      val = 5;
      rt.callProcedure(module, proc, io);
      if(val != 120)
        throw Exception();

      val = 10;
      rt.callProcedure(module, proc, io);
      if(val != 3628800)
        throw Exception();

      val = 20;
      rt.callProcedure(module, proc, io);
      if(val != 2432902008176640000LLU)
        throw Exception();
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

      bool testRuntime() {
        bool passed;

        passed = testFactorial();

        return passed;
      }

    }
  }
}
