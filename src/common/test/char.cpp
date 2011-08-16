#include <sstream>

#include "../../test/all.h"
#include "../char.h"
#include "../exception.h"

namespace {

  using namespace Ant::Common;
  using namespace Ant::Test;

  const String subj = "Ant::Common::Char";

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
      passed = false;
      in.str("\xD1");
      in.clear();

      try { in >> chr; }
      catch(const EndOfFileException&) { passed = true; }
      catch(...) {}
    }

    if(passed) {
      passed = false;
      in.str("\xD1\xD1");
      in.clear();

      try { in >> chr; }
      catch(const EncodingException&) { passed = true; }
      catch(...) {}
    }

    return printTestResult(subj, "input", passed);
  }

}

namespace Ant {
  namespace Common {
    namespace Test {

      bool testChar() {
        bool passed;

        passed = testInput();

        return passed;
      }

    }
  }
}
