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

  inline bool isDelimiter(Char chr) {
    return
      isWhitespace(chr) ||
      isLineCommentBegin(chr) ||
      isOpen(chr) ||
      isClose(chr); 
  }

  inline bool isDot(Char chr) {
    return chr == '.';
  }

}

namespace Ant {
  namespace Lang {
    using namespace Common;

    inline bool Lexer::updateLocation(Char chr) {
      if(chr == '\n')
        ++loc.line, loc.column = 1;
      else ++loc.column;
    }

    inline void Lexer::ungetChar(Char chr) {
      for(int i = chr.sequenceSize(); i; i--)
        in.unget();
    }

    void Lexer::skipWhitespaces() {
      bool insideLineComment = false;
      
      while(in.good()) {
        Char chr;
        in >> chr;

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

    Token Lexer::recognizeToken(const String &str) {
      if(str.length() == 1 && isDot(*str.begin()))
        return TOKEN_DOT;


      return TOKEN_SYMBOL;
    }

    Token Lexer::readToken() {
      skipWhitespaces();
      if(!in.good())
        return TOKEN_EOF;

      Char chr;
      String str;

      do {
        in >> chr;

        if(isDelimiter(chr))
          break;

        updateLocation(chr);

        str.push_back(chr);
      }
      while(in.good());

      if(!str.size()) {
        updateLocation(chr);
        return isOpen(chr) ? TOKEN_OPEN : TOKEN_CLOSE;
      }

      ungetChar(chr);

      return recognizeToken(str);
    }

  }
}
