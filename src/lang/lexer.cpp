#include "lexer.h"
#include "../common/char.h"

namespace {
  using namespace std;
  using namespace Ant::Lang;
  using namespace Ant::Common;

  inline bool updateLocation(Char chr, Location &loc) {
    if(chr == '\n')
      ++loc.line, loc.column = 1;
    else ++loc.column;
  }

  inline bool isWhitespace(Char chr) {
    return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n';
  }

  inline bool isLineCommentBegin(Char chr) {
    return chr == ';';
  }

  inline bool isLineCommentEnd(Char chr) {
    return chr == '\n';
  }

  inline void ungetChar(istream &in, Char chr) {
    for(int i = chr.sequenceSize(); i; i--)
      in.unget();
  }

  void skipWhitespaces(istream &in, Location &loc) {
    bool insideLineComment = false;

    while(in.good()) {
      Char chr;
      in >> chr;

      updateLocation(chr, loc);

      if(isLineCommentBegin(chr)) {
        insideLineComment = true;
        continue;
      }

      if(insideLineComment) {
        if(isLineCommentEnd(chr))
          insideLineComment = false;
        continue;
      }

      if(isWhitespace(chr))
        continue;

      ungetChar(in, chr);
      break;
    }
  }

}

namespace Ant {
  namespace Lang {

    Token Lexer::readToken() {
      while (in.good()) {
        skipWhitespaces(in, loc);

      }

      return TOKEN_EOF;
    }

  }
}
