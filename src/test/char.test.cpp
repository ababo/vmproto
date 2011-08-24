#include <sstream>

#include "../char.h"
#include "../exception.h"
#include "test.h"

namespace {

  using namespace Ant;
  using namespace Ant::Test;

  const String subj = "Ant::Char";

  bool testInput() {
    bool passed = true;
    std::istringstream in("\xD0\x9D\xD0\xB0\x20");
    Char chr;
    
    try {
      in >> chr, passed = chr == 0x041D;
      in >> chr, passed = passed && chr == 0x0430;
      in >> chr, passed = passed && chr == ' ';
      in >> chr, passed = passed && chr.isEOF();
    }
    catch (...) { passed = false; }

    if(passed) {
      in.str("\xD1");
      in.clear();

      ASSERT_THROW(in >> chr, EndOfFileException);
    }

    if(passed) {
      in.str("\xD1\xD1");
      in.clear();

      ASSERT_THROW(in >> chr, EncodingException);
    }

    return printTestResult(subj, "input", passed);
  }

}

namespace Ant {
  namespace Test {

    bool testChar() {
      bool passed;

      passed = testInput();

      return passed;
    }

  }
}
