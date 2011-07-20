#include "char.h"
#include "exception.h"
#include "utf8/source/utf8.h"

namespace Ant {
  namespace Common {

    using namespace std;
    using namespace utf8;

#define MAX_CP_BYTES 6

    istream &operator>>(istream &in, Char &chr) {
      char buf[MAX_CP_BYTES + 1];

      if(!in.good())
        throw EOFException();

      buf[0] = in.get();
      int len = internal::sequence_length(buf);
      for(int i = 1; i < len; i++) {
        if(!in.good())
          throw EOFException();

        buf[i] = in.get();
      }
      buf[len++] = 0;

      try { chr.c = peek_next(buf, buf + len); }
      catch (const invalid_utf8&) {
        throw EncodingException();
      }
      catch (const invalid_code_point&) {
        throw EncodingException();
      }

      return in;
    }

  }
}
