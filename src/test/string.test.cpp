#include "../exception.h"
#include "../string.h"
#include "test.h"

namespace {

  using namespace Ant;
  using namespace Ant::Test;

  const String subj = "Ant::String";

#define NON_ASCII "\xD0\x9D\xD0\xB0\x20\xD1\x80" \
  "\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xBE\xD0\xBC"

  bool testCtor() {
    bool passed = true;

    ASSERT_NOTHROW({String s = NON_ASCII;});
    ASSERT_THROW({String s = NON_ASCII "\xD0";}, EncodingException);

    return printTestResult(subj, "ctor", passed);
  }

  bool testLength() {
    String buf = NON_ASCII;

    return printTestResult(subj, "length", buf.length() == 10);
  }
}

namespace Ant {
  namespace Test {

    bool testString() {
      bool passed;

      passed = testCtor();
      passed = passed && testLength();

      return passed;
    }

  }
}
