#ifndef __UTIL_INCLUDED__
#define __UTIL_INCLUDED__

#include <iostream>

#include "exception.h"

#ifdef CONFIG_DEBUG
#define ASSERT(val) \
  if(!(val)) { \
    std::cerr << std::endl << "Assertion failed at location: "; \
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl << std::endl; \
    throw Ant::BugException(); \
  }
#else
#define ASSERT(val)
#endif

#define STRZ(arg) #arg

#endif // __UTIL_INCLUDED__
