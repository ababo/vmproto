#ifndef __VM_UTILS_INCLUDED__
#define __VM_UTILS_INCLUDED__

#include <stdint.h>
#include <ostream>
#include <istream>

namespace Ant {
  namespace VM {

    int writeMultibyteInteger(uint64_t value, std::ostream &out);
    int readMultibyteInteger(std::istream &in, uint64_t &value);

  }
}

#endif // __VM_UTILS_INCLUDED__
