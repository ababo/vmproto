#ifndef __TEST_ALL_INCLUDED__
#define __TEST_ALL_INCLUDED__

#include "../common/string.h"

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

    bool printTestResult(const Common::String subj,
                         const Common::String test,
                         bool testPassed);

    bool testAntOS();

  }
}

#endif // __TEST_ALL_INCLUDED__
