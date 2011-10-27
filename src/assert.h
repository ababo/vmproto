#ifndef __ASSERT_INCLUDED__
#define __ASSERT_INCLUDED__

#include <iostream>

#include "exception.h"

#ifdef CONFIG_DEBUG
#define ASSERT(val) \
  if(!(val)) { \
    cerr << endl << "Assertion failed at location: "; \
    cerr << __FILE__ << ":" << __LINE__ << endl << endl; \
    throw Ant::BugException(); \
  }
#else
#define ASSERT(val)
#endif

#endif // __ASSERT_INCLUDED__
