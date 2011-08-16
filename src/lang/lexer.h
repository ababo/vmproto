#ifndef __LANG_LEXER_INCLUDED__
#define __LANG_LEXER_INCLUDED__

#include <istream>
#include <stdint.h>

#include "../common/char.h"
#include "../common/string.h"

namespace Ant {
  namespace Lang {

    struct Location {
      Location() : line(1), column(1) {}
      Location(uint16_t line, uint16_t column) : line(line), column(column) {}

      bool operator==(const Location &loc) const {
        return line == loc.line && column == loc.column; }

      uint16_t line;
      uint16_t column;
    };

    enum Token {
      TOKEN_EOF = 0,
      TOKEN_OPEN,
      TOKEN_CLOSE,
      TOKEN_DOT,
      TOKEN_SYMBOL,
      TOKEN_STRING,
      TOKEN_POS_INT,
      TOKEN_NEG_INT,
      TOKEN_REAL
    };

    class Lexer {
    public:
      Lexer(std::istream &text, Location loc) : in(text), loc(loc) {}
      
      Token readToken();

      Location location() const { return loc; }
      Token token() const { return tok; }
      const Common::String &string() const { return str; }
      uint64_t posInt() const { return pint; }
      int64_t negInt() const { return nint; }
      double real() const { return rl; }

    protected:
      void updateLocation(Common::Char chr);
      void ungetChar(Common::Char chr);
      void skipWhitespaces();
      Token readNumberOrSymbol(const Common::String &str);
      Token readString();

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
