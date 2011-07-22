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
      TOKEN_EOF,
      TOKEN_OPEN,
      TOKEN_CLOSE,
      TOKEN_DOT,
      TOKEN_SYMBOL,
      TOKEN_STR_LIT,
      TOKEN_POS_INT,
      TOKEN_NEG_INT,
      TOKEN_REAL
    };

    class Lexer {
    public:
      Lexer(std::istream &text, Location location) : in(text) {}
      
      void nextToken();
      Token readToken();
      Location location() const { return loc; }
      Token token() const { return tok; }

      const Common::String &string() const { return str; }
      uint64_t posInt() const { return pint; }
      int64_t negInt() const { return nint; }
      double real() const { return rl; }

    protected:
      std::istream &in;
      Location loc;
      Token tok;

      Common::String str;
      uint64_t pint;
      int64_t nint;
      double rl;
    };
  }
}

#endif // __LANG_LEXER_INCLUDED__
