#ifndef __VM_UTILS_INCLUDED__
#define __VM_UTILS_INCLUDED__

#include <cstddef>
#include <stdint.h>
#include <ostream>
#include <istream>

namespace Ant {
  namespace VM {

    extern const uint64_t MB_MASKS[9];

#define MB_UINT_MAX(size) MB_MASKS[(size) - 1]
#define MB_INT_MAX(size) (MB_MASKS[(size) - 1] >> 1)
#define MB_INT_MIN(size) (-(MB_MASKS[(size) - 1] >> 1) - 1)

    size_t writeMBUInt(uint64_t value, std::ostream &out);
    size_t readMBUInt(std::istream &in, uint64_t &value);
    size_t writeMBInt(int64_t value, std::ostream &out);
    size_t readMBInt(std::istream &in, int64_t &value);

  }
}

#endif // __VM_UTILS_INCLUDED__
