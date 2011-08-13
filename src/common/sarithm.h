#ifndef __COMMON_SARITHM_INCLUDED__
#define __COMMON_SARITHM_INCLUDED__

#include <stdint.h>

namespace Ant {
  namespace Common {

    bool safeToAdd(uint32_t a, uint32_t b) {
      return static_cast<uint32_t>(-1) - a >= b; }
    bool safeToAdd(uint64_t a, uint64_t b) {
      return static_cast<uint64_t>(-1) - a >= b; }

    bool safeToMultiply(uint32_t a, uint32_t b) {
      return a ? static_cast<uint32_t>(-1) / a >= b : true; }
    bool safeToMultiply(uint64_t a, uint64_t b) {
      return a ? static_cast<uint64_t>(-1) / a >= b : true; }

  }
}

#endif // __COMMON_SARITHM_INCLUDED__
