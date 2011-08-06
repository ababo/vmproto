#include "../../common/exception.h"
#include "../../common/string.h"
#include "../../test/all.h"
#include "../module.h"
#include "all.h"

namespace {

  using namespace std;
  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::Common;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::Module";

  bool testFactorial() {
    StaticVariable<1, 8, 0, 0> io;
    uint64_t &val = *reinterpret_cast<uint64_t*>(io.elts[0].bytes);
    bool passed = true;
    ProcId proc = 0;
    Module module;

    try {
      createFactorialModule(module);
      module.unpack();

      val = 0;
      module.callFunc(proc, io);
      if(val != 1)
        throw Exception();

      val = 1;
      module.callFunc(proc, io);
      if(val != 1)
        throw Exception();

      val = 5;
      module.callFunc(proc, io);
      if(val != 120)
        throw Exception();

      val = 10;
      module.callFunc(proc, io);
      if(val != 3628800)
        throw Exception();

      val = 20;
      module.callFunc(proc, io);
      if(val != 2432902008176640000LLU)
        throw Exception();

      module.drop();
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "factorial", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testModule() {
        bool passed;

        passed = testFactorial();

        return passed;
      }

    }
  }
}
