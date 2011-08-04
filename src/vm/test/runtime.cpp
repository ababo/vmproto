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
    bool passed = true;

    vector<Variable*> ioframe;
    StaticVariable<8> var;
    ioframe.push_back(&var);
    uint64_t &val = *reinterpret_cast<uint64_t*>(var.bytes);
    ProcId proc = 0;
    
    try {
      Runtime &rt = Runtime::instance();
      const UUID &id = createFactorialModule(rt);
      rt.unpackModule(id);

      val = 0;
      rt.callProcedure(id, proc, ioframe);
      if(val != 1)
        throw Exception();

      val = 1;
      rt.callProcedure(id, proc, ioframe);
      if(val != 1)
        throw Exception();

      val = 5;
      rt.callProcedure(id, proc, ioframe);
      if(val != 120)
        throw Exception();

      val = 10;
      rt.callProcedure(id, proc, ioframe);
      if(val != 3628800)
        throw Exception();

      val = 20;
      rt.callProcedure(id, proc, ioframe);
      if(val != 2432902008176640000LLU)
        throw Exception();
    }
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
