#include "char.h"
#include "exception.h"
#include "utf8/source/utf8.h"

namespace Ant {
  namespace Common {

    Char &Char::operator=(uint32_t cp) {
      if(!utf8::internal::is_code_point_valid(cp))
        throw CodePointException();

      c = cp;
    }

    size_t Char::sequenceSize() const {
      if(c < 0x80)
        return 1;
      if(c < 0x800)
        return 2;
      if(c < 0x10000)
        return 3;
      return 4;
    }

    std::istream &operator>>(std::istream &in, Char &chr) {
      char buf[4 + 1];

      if(!in.good())
        throw EndOfFileException();

      buf[0] = in.get();
      int len = utf8::internal::sequence_length(buf);
      for(int i = 1; i < len; i++) {
        if(!in.good())
          throw EndOfFileException();

        buf[i] = in.get();
      }
      buf[len++] = 0;

      try { chr.c = utf8::peek_next(buf, buf + len); }
      catch (const utf8::invalid_utf8&) {
        throw EncodingException();
      }
      catch (const utf8::invalid_code_point&) {
        throw CodePointException();
      }

      return in;
    }

  }
}
