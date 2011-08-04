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

  static const uint64_t mbInts[] = {
    0x7F, 0x3FFF, 0x1FFFFF, 0xFFFFFFF, 0x7FFFFFFFFLLU, 0x3FFFFFFFFFFLLU,
    0x1FFFFFFFFFFFFLLU, 0xFFFFFFFFFFFFFFLLU, 0xFFFFFFFFFFFFFFFFLLU
  };

  bool testMultibyteIntegers() {
    bool passed = true;
    stringstream io;
    uint64_t val;

    try {
      for(int i = 0; i < 9; i++) {
        passed = passed && writeMultibyteInteger(mbInts[i], io) == i + 1;
        if(i < 8)
          passed = passed && writeMultibyteInteger(mbInts[i] + 1, io) == i + 2;
      }

      for(int i = 0; i < 9; i++) {
        passed = passed && readMultibyteInteger(io, val) == i + 1;
        passed = passed && val == mbInts[i];
        if(i < 8) {
          passed = passed && readMultibyteInteger(io, val) == i + 2;
          passed = passed && val == mbInts[i] + 1;
        }
      }
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "multibyteIntegers", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testUtils() {
        bool passed;

        passed = testMultibyteIntegers();

        return passed;
      }

    }
  }
}
