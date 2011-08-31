#ifndef __UUID_INCLUDED__
#define __UUID_INCLUDED__

#include <string.h>

namespace Ant {

#define UUID_SIZE 16

  class UUID {
  public:
    UUID() { memset(dat, 0, sizeof(dat)); }

    const unsigned char *data() const { return dat; }

    bool operator<(const UUID &uuid) const {
      return memcmp(dat, uuid.dat, sizeof(dat));
    }

    UUID &generate();

  protected:
    unsigned char dat[UUID_SIZE];
  };

}

#endif // __UUID_INCLUDED__
