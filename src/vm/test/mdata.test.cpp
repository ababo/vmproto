#include <string.h>

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
      SVariable<8, 0, 0> io;
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

#define QSORT_ARR_COUNT 16

  bool testQSort() {
    bool passed = true;
    Module module;

    try {
      const int64_t in[QSORT_ARR_COUNT] = {
	123, 34, -23, 0, 876, 34, 268, 994, -74, -222, 43, 13, -1, 6, 78, 56 };
      const int64_t out[QSORT_ARR_COUNT] = {
	-222, -74, -23, -1, 0, 6, 13, 34, 34, 43, 56, 78, 123, 268, 876, 994 };

      SVariable<16, 1, 0> io;
      SVContainer<8, 0, 0, QSORT_ARR_COUNT, true, true> a;
      *reinterpret_cast<uint64_t*>(&io.elts[0].bytes[0]) = 0;
      *reinterpret_cast<uint64_t*>(&io.elts[0].bytes[8]) = QSORT_ARR_COUNT - 1;
      a.refCount = 1, a.eltCount = QSORT_ARR_COUNT;
      io.elts[0].vrefs[0] = &a.var;

      ProcId proc = 1;
      createQSortModule(module);
      module.unpack();

      memcpy(a.var.elts[0].bytes, in, sizeof(in));
      module.callProc(proc, io);
      if(memcmp(a.var.elts[0].bytes, out, sizeof(out)))
        throw Exception();
    }
    catch(...) { passed = false; }

    IGNORE_THROW(module.drop());

    return printTestResult(subj, "qsort", passed);
  }

  bool testEH() {
    bool passed = true;
    Module module;

    try {
      SVariable<8, 0, 0> io;
      uint64_t &val = *reinterpret_cast<uint64_t*>(io.elts[0].bytes);
      ProcId proc = 1;

      createEHTestModule(module);
      module.unpack();

      val = 0;
      module.callProc(proc, io);
      if(val != -1)
        throw Exception();

      val = 1;
      module.callProc(proc, io);
      if(val != -2)
        throw Exception();

      val = 2;
      ASSERT_THROW({module.callProc(proc, io);}, RuntimeException);
    }
    catch(...) { passed = false; }

    IGNORE_THROW(module.drop());

    return printTestResult(subj, "EH", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testModuleData() {
        bool passed;

        passed = testFactorial();
	passed = passed && testQSort();
        // passed = passed && testEH();

        return passed;
      }

    }
  }
}
