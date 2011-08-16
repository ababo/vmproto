#ifndef __COMMON_CHAR_INCLUDED__
#define __COMMON_CHAR_INCLUDED__

#include <istream>
#include <stdint.h>

namespace Ant {
  namespace Common {

    class Char {
      friend std::istream &operator>>(std::istream &in, Char &chr);
    public:
      Char() : cp(0) {}
      Char(uint32_t cp) : cp(cp) {}

      operator uint32_t() const { return cp; }
      Char &operator=(uint32_t cp) { this->cp = cp; return *this; }

      bool isEOF() const;
      bool isValid() const;
      size_t seqSize() const;

      Char read(std::istream &in);

    protected:
      uint32_t cp;
    };

  }
}

#endif // __COMMON_CHAR_INCLUDED__
