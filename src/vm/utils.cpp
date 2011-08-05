#include <cstdio>

#include "utils.h"
#include "../common/exception.h"

namespace Ant {
  namespace VM {

    using namespace Ant::Common;

    int writeMultibyteInteger(uint64_t value, std::ostream &out) {
      uint8_t byte;
      int count = 0;

      do {
        bool last = ++count == 9;

        byte = value & (last ? 0xFF : 0x7F);

        if(value >>= (last ? 8 : 7))
          out.put(byte | 0x80);
        else out.put(byte);

        if(out.bad())
          throw IOException();
      }
      while(value);

      return count;
    }

    int readMultibyteInteger(std::istream &in, uint64_t &value) {
      int chr, count = 0;
      uint64_t tmp, val = 0;

      for(;;) {
        chr = in.get();
        if(chr == EOF)
          throw EndOfFileException();
        if(in.bad())
          throw IOException();

        bool last = count == 8;

        tmp = last ? chr : (chr & 0x7F);
        tmp <<= 7 * count++;
        val |= tmp;

        if(last || !(chr & 0x80))
          break;
      };

      value = val;
      return count;
    }

  }
}
