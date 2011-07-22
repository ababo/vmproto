#ifndef __COMMON_CHAR_INCLUDED__
#define __COMMON_CHAR_INCLUDED__

#include <stdint.h>
#include <istream>

namespace Ant {
  namespace Common {

    class Char {
      friend std::istream &operator>>(std::istream &in, Char &chr);
    public:
      Char() : c(0) {}
      Char(uint32_t cp) { *this = cp; }

      operator uint32_t() const { return c; }
      Char &operator=(uint32_t cp);

    protected:
      uint32_t c;
    };

  }
}

#endif // __COMMON_CHAR_INCLUDED__
