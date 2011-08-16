#include <cstdio>
#include <sstream>

#include "../common/exception.h"
#include "lexer.h"

namespace {

  using namespace Ant::Common;

  inline bool isWhitespace(Char chr) {
    return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n';
  }

  inline bool isLineCommentBegin(Char chr) {
    return chr == ';';
  }

  inline bool isLineCommentEnd(Char chr) {
    return chr == '\n';
  }

  inline bool isOpen(Char chr) {
    return chr == '(';
  }

  inline bool isClose(Char chr) {
    return chr == ')';
  }

  inline bool isStringDelimiter(Char chr) {
    return chr == '"';
  }

  inline bool isDelimiter(Char chr) {
    return
      isWhitespace(chr) ||
      isLineCommentBegin(chr) ||
      isOpen(chr) ||
      isClose(chr) ||
      isStringDelimiter(chr);
  }

  inline bool isDot(Char chr) {
    return chr == '.';
  }

  inline bool isEscapeChar(Char chr) {
    return chr == '\\';
  }

  int signIfInteger(const std::string &str) {
    if(str.empty())
      return 0;

    int i = 0, sign = 1;
    if(str[0] == '+')
      ++i;
    else if(str[0] == '-')
      sign = -1, ++i;

    if(i == str.length())
      return 0;

    bool zero = true;
    for(; i < str.length(); i++)
      if(str[i] >= '1' && str[i] <= '9')
        zero = false;
      else if(str[i] != '0')
        return 0;

    if(zero)
      sign = 1;

    return sign;
  }

}

namespace Ant {
  namespace Lang {
    using namespace Common;

    void Lexer::updateLocation(Char chr) {
      if(chr == '\n')
        ++loc.line, loc.column = 1;
      else ++loc.column;
    }

    void Lexer::ungetChar(Char chr) {
      for(int i = chr.seqSize(); i; i--)
        in.unget();
    }

    void Lexer::skipWhitespaces() {
      bool insideLineComment = false;
      Char chr;

      while(!chr.read(in).isEOF()) {
        if(isLineCommentBegin(chr)) {
          insideLineComment = true;
          updateLocation(chr);
          continue;
        }

        if(insideLineComment) {
          if(isLineCommentEnd(chr))
            insideLineComment = false;
          updateLocation(chr);
          continue;
        }

        if(isWhitespace(chr)) {
          updateLocation(chr);
          continue;
        }

        ungetChar(chr);
        break;
      }
    }

    Token Lexer::readString() {
      Char chr;
      String str;

      while(!chr.read(in).isEOF()) {
        updateLocation(chr);

        if(isEscapeChar(chr)) {
          if(chr.read(in).isEOF())
            break;

          if(isEscapeChar(chr) || isStringDelimiter(chr)) {
            updateLocation(chr);
            str.push_back(chr);
            continue;
          }

          throw EscapeCharException();
        }

        if(isStringDelimiter(chr)) {
          this->str = str;
          return TOKEN_STRING;
        }

        str.push_back(chr);
      }

      throw EndOfFileException();
    }

    Token Lexer::readNumberOrSymbol(const String &str) {
      std::istringstream in(str);

      int sign = signIfInteger(str);
      
      if(sign > 0) {
        uint64_t pint;
        if((in >> pint).fail())
          throw RangeException();

        loc.column += str.size();
        this->pint = pint;
        return TOKEN_POS_INT;
      }

      if(sign < 0) {
        int64_t nint;
        if((in >> nint).fail())
          throw RangeException();

        loc.column += str.size();
        this->nint = nint;
        return TOKEN_NEG_INT;
      }

      double rl;
      if(!(in >> rl).fail() && in.eof()) {
        loc.column += str.size();
        this->rl = rl;
        return TOKEN_REAL;
      }

      loc.column += str.length();
      this->str = str;
      return TOKEN_SYMBOL;
    }

    Token Lexer::readToken() {
      Char chr;
      String str;

      skipWhitespaces();
      if(chr.read(in).isEOF())
        return tok = TOKEN_EOF;

      do {
        if(isDelimiter(chr))
          break;

        str.push_back(chr);
      }
      while(!chr.read(in).isEOF());

      if(!str.size()) {
        updateLocation(chr);

        if(isStringDelimiter(chr))
          return tok = readString();

        return tok = isOpen(chr) ? TOKEN_OPEN : TOKEN_CLOSE;
      }

      if(!chr.isEOF())
        ungetChar(chr);

      if(str.length() == 1 && isDot(*str.begin())) {
        updateLocation(chr);
        return tok = TOKEN_DOT;
      }

      return tok = readNumberOrSymbol(str);
    }

  }
}
