#include <sstream>

#include "../../common/string.h"
#include "../../test/all.h"
#include "../utils.h"

namespace {

  using namespace std;
  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::Common;

  const String subj = "Ant::VM::{Utils}";

  const uint64_t MB_MASKS[9] = {
    0x7F, 0x3FFF, 0x1FFFFF, 0xFFFFFFF, 0x7FFFFFFFFLLU, 0x3FFFFFFFFFFLLU,
    0x1FFFFFFFFFFFFLLU, 0xFFFFFFFFFFFFFFLLU, 0xFFFFFFFFFFFFFFFFLLU
  };

  bool testMBUInts() {
    stringstream io;
    uint64_t val;
    bool passed;

    try {
      passed = writeMBUInt(0, io) == 1;
      for(int i = 0; passed && i < 9; i++) {
        passed = writeMBUInt(MB_MASKS[i], io) == i + 1;
        if(passed && i < 8)
          passed = writeMBUInt(MB_MASKS[i] + 1, io) == i + 2;
      }

      passed = readMBUInt(io, val) == 1 && !val;
      for(int i = 0; passed && i < 9; i++) {
        passed = readMBUInt(io, val) == i + 1;
        passed = passed && val == MB_MASKS[i];
        if(passed && i < 8) {
          passed = readMBUInt(io, val) == i + 2;
          passed = passed && val == MB_MASKS[i] + 1;
        }
      }
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "mbUInts", passed);
  }

  bool testMBInts() {
    stringstream io;
    int64_t tmp, val;
    bool passed;

    try {
      passed = writeMBInt(0, io) == 1;
      for(int i = 0; passed && i < 9; i++) {
        tmp = MB_MASKS[i] >> 1;
        passed = writeMBInt(tmp, io) == i + 1;
        passed = passed && writeMBInt(-tmp - 1, io) == i + 1;
        if(passed && i < 8) {
          passed = writeMBInt(tmp + 1, io) == i + 2;
          passed = passed && writeMBInt(-tmp - 2, io) == i + 2;
        }
      }

      passed = readMBInt(io, val) == 1 && !val;
      for(int i = 0; passed && i < 9; i++) {
        tmp = MB_MASKS[i] >> 1;
        passed = readMBInt(io, val) == i + 1;
        passed = passed && val == tmp;
        passed = passed && readMBInt(io, val) == i + 1;
        passed = passed && val == -tmp - 1;
        if(passed && i < 8) {
          passed = readMBInt(io, val) == i + 2;
          passed = passed && val == tmp + 1;
          passed = passed && readMBInt(io, val) == i + 2;
          passed = passed && val == -tmp - 2;
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
