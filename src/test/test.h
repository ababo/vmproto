#ifndef __TEST_INCLUDED__
#define __TEST_INCLUDED__

#include "../string.h"

#define ASSERT_THROW(code, exception) \
  if(passed) { \
    try { code; passed = false; } \
    catch(const exception&) {} \
    catch(...) { passed = false; } \
  }

#define ASSERT_NOTHROW(code) \
  if(passed) { \
    try { code; } \
    catch(...) { passed = false; } \
  }

#define IGNORE_THROW(code) \
  try { code; } \
  catch(...) {}

namespace Ant {
  namespace Test {

    bool printTestResult(const String subj,
                         const String test,
                         bool testPassed);

    bool testChar();
    bool testString();

    bool testAntOS();

  }
}

#endif // __TEST_INCLUDED__
