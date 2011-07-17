#ifndef __LANG_LEXER_INCLUDED__
#define __LANG_LEXER_INCLUDED__

#include <stdint.h>
#include <istream>

#include "../common/string.h"

namespace Ant {
  namespace Lang {

    struct Location {
      uint16_t line;
      uint16_t column;
    };

    enum Token {
      TOKEN_ERR = 0,
      TOKEN_EOF,
      TOKEN_LEFT_BR,
      TOKEN_RIGHT_BR,
      TOKEN_SYMBOL,
      TOKEN_STR_LIT,
      TOKEN_NEG_INT,
      TOKEN_POS_INT,
      TOKEN_REAL,
    };

    class Lexer {
    public:
      Lexer(std::istream &text, Location location) : in(text) {}
      
      void nextToken();
      Token readToken();
      Location location() { return loc; }

      const Common::String &getString() { return str; }
      uint64_t getPosInt() { return pint; }
      int64_t getNegInt() { return nint; }
      double getReal() { return real; }

    protected:
      std::istream &in;
      Location loc;

      Common::String str;
      uint64_t pint;
      int64_t nint;
      double real;
    };
  }
}

#endif // __LANG_LEXER_INCLUDED__
