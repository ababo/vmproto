#include "all.h"
#include "../string.h"
#include "../exception.h"
#include "../../test/all.h"

namespace {

  using namespace Ant::Common;
  using namespace Ant::Test;

  const String subj = "Ant::Common::String";

  const char *NON_ASCII = "\xD0\x9D\xD0\xB0\x20\xD1\x80"
    "\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xBE\xD0\xBC";

  const char *NON_UTF8 = "\xD0\x9D\xD0\xB0\x20\xD1\x80"
    "\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xBE\xD0\xD0";

  bool testSanity() {
    bool passed = true;

    try { String s = NON_ASCII; }
    catch (...) { passed = false; }

    passed = false;

    try { String s = NON_UTF8; }
    catch (const EncodingException&) { passed = true; }
    catch (...) { }

    return printTestResult(subj, "sanity", passed);
  }

  bool testLength() {
    String buf = NON_ASCII;

    return printTestResult(subj, "length", buf.length() == 18);
  }
}

namespace Ant {
  namespace Common {
    namespace Test {

      bool testString() {
        return
          testSanity() &&
          testLength();
      }

    }
  }
}
