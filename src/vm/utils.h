#ifndef __VM_UTILS_INCLUDED__
#define __VM_UTILS_INCLUDED__

#include <cstddef>
#include <stdint.h>
#include <ostream>
#include <istream>

namespace Ant {
  namespace VM {

    size_t writeMultibyteInteger(uint64_t value, std::ostream &out);
    size_t readMultibyteInteger(std::istream &in, uint64_t &value);

  }
}

#endif // __VM_UTILS_INCLUDED__
