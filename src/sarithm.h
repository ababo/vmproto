#ifndef __SARITHM_INCLUDED__
#define __SARITHM_INCLUDED__

#include <stdint.h>

namespace Ant {

  bool safeToAdd(uint32_t a, uint32_t b) { return uint32_t(-1) - a >= b; }
  bool safeToAdd(uint64_t a, uint64_t b) { return uint64_t(-1) - a >= b; }

  bool safeToMultiply(uint32_t a, uint32_t b) {
    return a ? static_cast<uint32_t>(-1) / a >= b : true;
  }
  bool safeToMultiply(uint64_t a, uint64_t b) {
    return a ? static_cast<uint64_t>(-1) / a >= b : true;
  }

}

#endif // __SARITHM_INCLUDED__
