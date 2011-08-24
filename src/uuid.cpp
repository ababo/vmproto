#ifdef PLATFORM_LINUX
#include <uuid/uuid.h>
#define GENERATE(data) uuid_generate(dat);
#endif

#include "uuid.h"

namespace Ant {

  bool UUID::operator<(const UUID &uuid) const {
    for(int i = UUID_SIZE - 1; i >= 0; i--)
      if(uuid.dat[i] != dat[i])
        return uuid.dat[i] > dat[i];

    return false;
  }

  UUID &UUID::generate() {
    GENERATE(dat);
    return *this;
  }

}
