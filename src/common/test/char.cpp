#include <sstream>

#include "all.h"
#include "../char.h"
#include "../exception.h"
#include "../../test/all.h"

namespace {

  using namespace Ant::Common;
  using namespace Ant::Test;

  const String subj = "Ant::Common::Char";

  bool testInput() {
    bool passed = true;
    
    Char chr;    
    std::stringstream in("\xD0\x9D\xD0\xB0\x20\xD1");
    
    try {
      in >> chr, passed = chr == 0x041F;
      in >> chr, passed = passed && chr == 0x043E;
      in >> chr, passed = passed && chr == ' ';
    }
    catch (...) { passed = false; }

    if(passed) {
      passed = false;

      try { in >> chr; }
      catch(const EOFException&) { passed = true; }
      catch(...) {}
    }

    in.str("\xD1\xD1");

    if(passed) {
      passed = false;

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
        return
          testInput();
      }

    }
  }
}
