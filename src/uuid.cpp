#ifdef PLATFORM_LINUX
#include <uuid/uuid.h>
#define GENERATE(data) uuid_generate(dat);
#endif

#include "uuid.h"

namespace Ant {

  UUID &UUID::generate() {
    GENERATE(dat);
    return *this;
  }

}
