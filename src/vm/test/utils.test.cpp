#include <sstream>

#include "../../string.h"
#include "../../test/test.h"
#include "../utils.h"

namespace {

  using namespace std;
  using namespace Ant;
  using namespace Ant::VM;
  using namespace Ant::Test;

  const String subj = "Ant::VM::{Utils}";

  bool testMBUInts() {
    stringstream io;
    uint64_t val;
    bool passed;

    try {
      passed = writeMBUInt(0, io) == 1;
      for(int i = 1; passed && i <= 9; i++) {
        passed = writeMBUInt(MB_UINT_MAX(i), io) == i;
        if(passed && i < 9)
          passed = writeMBUInt(MB_UINT_MAX(i) + 1, io) == i + 1;
      }

      passed = readMBUInt(io, val) == 1 && !val;
      for(int i = 1; passed && i <= 9; i++) {
        passed = readMBUInt(io, val) == i;
        passed = passed && val == MB_UINT_MAX(i);
        if(passed && i < 9) {
          passed = readMBUInt(io, val) == i + 1;
          passed = passed && val == MB_UINT_MAX(i) + 1;
        }
      }
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "mbUInts", passed);
  }

  bool testMBInts() {
    stringstream io;
    int64_t val;
    bool passed;

    try {
      passed = writeMBInt(0, io) == 1;
      for(int i = 1; passed && i <= 9; i++) {
        passed = writeMBInt(MB_INT_MAX(i), io) == i;
        passed = passed && writeMBInt(MB_INT_MIN(i), io) == i;
        if(passed && i < 9) {
          passed = writeMBInt(MB_INT_MAX(i) + 1, io) == i + 1;
          passed = passed && writeMBInt(MB_INT_MIN(i) - 1, io) == i + 1;
        }
      }

      passed = readMBInt(io, val) == 1 && !val;
      for(int i = 1; passed && i <= 9; i++) {
        passed = readMBInt(io, val) == i;
        passed = passed && val == MB_INT_MAX(i);
        passed = passed && readMBInt(io, val) == i;
        passed = passed && val == MB_INT_MIN(i);
        if(passed && i < 9) {
          passed = readMBInt(io, val) == i + 1;
          passed = passed && val == MB_INT_MAX(i) + 1;
          passed = passed && readMBInt(io, val) == i + 1;
          passed = passed && val == MB_INT_MIN(i) - 1;
        }
      }
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "mbInts", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testUtils() {
        bool passed;

        passed = testMBUInts();
        passed = passed && testMBInts();

        return passed;
      }

    }
  }
}
