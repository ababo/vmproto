#include "../../common/string.h"
#include "../../test/all.h"
#include "../utils.h"

#include <sstream>

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
    bool passed = true;
    stringstream io;
    uint64_t val;

    try {
      for(int i = 0; i < 9; i++) {
        passed = passed && writeMBUInt(MB_MASKS[i], io) == i + 1;
        if(i < 8)
          passed = passed && writeMBUInt(MB_MASKS[i] + 1, io) == i + 2;
      }

      for(int i = 0; i < 9; i++) {
        passed = passed && readMBUInt(io, val) == i + 1;
        passed = passed && val == MB_MASKS[i];
        if(i < 8) {
          passed = passed && readMBUInt(io, val) == i + 2;
          passed = passed && val == MB_MASKS[i] + 1;
        }
      }
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "mbUInts", passed);
  }

  bool testMBInts() {
    bool passed = true;
    stringstream io;
    int64_t tmp, val;

    try {
      for(int i = 0; i < 9; i++) {
        tmp = MB_MASKS[i] >> 1;
        passed = passed && writeMBInt(tmp, io) == i + 1;
        passed = passed && writeMBInt(-tmp - 1, io) == i + 1;
        if(i < 8) {
          passed = passed && writeMBInt(tmp + 1, io) == i + 2;
          passed = passed && writeMBInt(-tmp - 2, io) == i + 2;
        }
      }

      for(int i = 0; i < 9; i++) {
        tmp = MB_MASKS[i] >> 1;
        passed = passed && readMBInt(io, val) == i + 1;
        passed = passed && val == tmp;
        passed = passed && readMBInt(io, val) == i + 1;
        passed = passed && val == -tmp - 1;
        if(i < 8) {
          passed = passed && readMBInt(io, val) == i + 2;
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
