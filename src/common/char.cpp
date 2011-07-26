#include <cstdio>

#include "char.h"
#include "exception.h"
#include "utf8/source/utf8.h"

namespace Ant {
  namespace Common {

    bool Char::isEOF() const {
      return cp == static_cast<uint32_t>(EOF);
    }

    bool Char::isValid() const {
      return utf8::internal::is_code_point_valid(cp);
    }

    size_t Char::seqSize() const {
      if(cp < 0x80)
        return 1;
      if(cp < 0x800)
        return 2;
      if(cp < 0x10000)
        return 3;
      return 4;
    }

    Char Char::read(std::istream &in) {
      int chr = in.get();
      if(chr == EOF) {
        cp = static_cast<uint32_t>(EOF);
        return *this;
      }

      char buf[4 + 1];
      buf[0] = (char)chr;
      int len = utf8::internal::sequence_length(buf);

      for(int i = 1; i < len; i++) {
        chr = in.get();
        if(chr == EOF)
          throw EndOfFileException();

        buf[i] = (char)chr;
      }
      buf[len++] = 0;

      try { cp = utf8::peek_next(buf, buf + len); }
      catch (const utf8::invalid_utf8&) {
        throw EncodingException();
      }
      catch (const utf8::invalid_code_point&) {
        throw CodePointException();
      }

      return *this;
    }

    std::istream &operator>>(std::istream &in, Char &chr) {
      chr.read(in);
      return in;
    }

  }
}
