#include <cstdio>

#include "utils.h"
#include "../common/exception.h"

namespace {

  using namespace Ant::Common;

  size_t writeMBUIntEx(uint64_t value, std::ostream &out, bool extend) {
    uint8_t byte;
    size_t size = 0;

    do {
      bool last = ++size == 9;

      if(last)
        extend = false;

      byte = value & (last ? 0xFF : 0x7F);

      if((value >>= (last ? 8 : 7)) || extend)
        out.put(byte | 0x80);
      else out.put(byte);

      if(out.bad())
        throw IOException();
    }
    while(value);

    return size;
  }

}

namespace Ant {
  namespace VM {

    using namespace Ant::Common;

    size_t writeMBUInt(uint64_t value, std::ostream &out) {
      return writeMBUIntEx(value, out, false);
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
      bool neg = value < 0;
      bool extend = false;
      uint8_t extention;

      uint8_t byte = uint8_t(value >> 56);
      if(!byte || byte == 0xFF)
        for(int i = 49; i >= 0; i -= 7) {
          byte = uint8_t(value >> i) & 0x7F;

          if(!i || byte || byte != 0x7F) {
            bool hbit = byte & 0x40;
            if(neg && !hbit)
              extend = true, extention = 0x7F;
            else if(!neg && hbit)
              extend = true, extention = 0;

            int offset = 57 - i;
            value <<= offset >>= offset;
            break;
          }
        }

      size_t size = writeMBUIntEx(uint64_t(value), out, extend);
      if(extend) {
        out.put(extention);

        if(out.bad())
          throw IOException();
      }

      return size + size_t(extend);
    }

    size_t readMBInt(std::istream &in, int64_t &value) {

      return 0;
    }

  }
}
