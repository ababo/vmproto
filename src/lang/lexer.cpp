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

    bool Lexer::updateLocation(Char chr) {
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

    Token Lexer::recognizeToken(const String &str) {
      if(str.length() == 1 && isDot(*str.begin()))
        return TOKEN_DOT;


      this->str = str;
      return TOKEN_SYMBOL;
    }

    Token Lexer::readToken() {
      Char chr;
      String str;

      skipWhitespaces();
      if(chr.read(in).isEOF())
        return TOKEN_EOF;

      do {
        if(isDelimiter(chr))
          break;

        updateLocation(chr);
        str.push_back(chr);
      }
      while(!chr.read(in).isEOF());

      if(!str.size()) {
        updateLocation(chr);
        return isOpen(chr) ? TOKEN_OPEN : TOKEN_CLOSE;
      }

      if(!chr.isEOF())
        ungetChar(chr);

      return recognizeToken(str);
    }

  }
}
