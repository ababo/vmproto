#include <cstdio>

#include "utils.h"
#include "../common/exception.h"

namespace Ant {
  namespace VM {

    using namespace Ant::Common;

    size_t writeMBUInt(uint64_t value, std::ostream &out) {
      uint8_t byte;
      size_t size = 0;

      do {
        bool last = ++size == 9;

        byte = value & (last ? 0xFF : 0x7F);

        if(value >>= (last ? 8 : 7))
          out.put(byte | 0x80);
        else out.put(byte);

        if(out.bad())
          throw IOException();
      }
      while(value);

      return size;
    }

    size_t readMBUInt(std::istream &in, uint64_t &value) { 
      uint64_t tmp, val = 0;
      size_t size = 0;
      int chr;

      for(;;) {
        chr = in.get();
        if(chr == EOF)
          throw EndOfFileException();
        if(in.bad())
          throw IOException();

        bool last = size == 8;

        tmp = last ? chr : (chr & 0x7F);
        tmp <<= 7 * size++;
        val |= tmp;

        if(last || !(chr & 0x80))
          break;
      };

      value = val;
      return size;
    }

    size_t writeMBInt(int64_t value, std::ostream &out) {

      return 0;
    }

    size_t readMBInt(std::istream &in, int64_t &value) {

      return 0;
    }

  }
}
