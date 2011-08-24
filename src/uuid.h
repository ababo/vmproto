#ifndef __UUID_INCLUDED__
#define __UUID_INCLUDED__

#include <algorithm>

namespace Ant {

#define UUID_SIZE 16

  class UUID {
  public:
    UUID() { std::fill(dat, dat + sizeof(dat), 0); }

    const unsigned char *data() const { return dat; }

    bool operator<(const UUID &uuid) const {
      for(int i = UUID_SIZE - 1; i >= 0; i--)
        if(uuid.dat[i] != dat[i])
          return uuid.dat[i] > dat[i];
      return false;
    }

    UUID &generate();

  protected:
    unsigned char dat[UUID_SIZE];
  };

}

#endif // __UUID_INCLUDED__
