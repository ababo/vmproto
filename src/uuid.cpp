#ifdef PLATFORM_LINUX
#include <uuid/uuid.h>
#define GENERATE(data) uuid_generate(dat);
#define STR(data, buf) uuid_unparse(dat, buf);
#endif

#include "uuid.h"

namespace Ant {

  UUID &UUID::generate() {
    GENERATE(dat);
    return *this;
  }

  String UUID::str() const {
    char buf[37];
    STR(dat, buf);
    return String(buf);
  }

}
