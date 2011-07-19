#include <sstream>
#include <cmath>

#include "../../test/all.h"
#include "../lexer.h"

namespace {

  using namespace std;
  using namespace Ant::Test;
  using namespace Ant::Common;
  using namespace Ant::Lang;

  const String subj = "Ant::Lang::Lexer";

#define PREPARE_LEXER(buf_content) \
  const String buf = buf_content; \
  stringstream in(buf); \
  Location loc = { 0, 0 }; \
  Lexer lex(in, loc);

  bool testWhitespaces() {
    PREPARE_LEXER("; comment \n \r\t\n\r\n");

    bool passed = lex.readToken() == TOKEN_EOF;
    return printTestResult(subj, "whitespaces", passed);
  }

  bool testDelimiters() {
    PREPARE_LEXER(".)).. .(.");

    bool passed =
      lex.readToken() == TOKEN_DOT &&
      lex.readToken() == TOKEN_CLOSE &&
      lex.readToken() == TOKEN_CLOSE &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == ".." &&
      lex.readToken() == TOKEN_DOT &&
      lex.readToken() == TOKEN_OPEN &&
      lex.readToken() == TOKEN_DOT &&
      lex.readToken() == TOKEN_EOF;
    
    return printTestResult(subj, "delimiters", passed);
  }

#define LONG_SYMBOL1 "H!e@l#l$o%_^w&o*r~l<d>?...+-/"
#define LONG_SYMBOL2 "\65\45\208\191\208\190\45\209\128\209\131\209\129" \
  "\209\129\208\186\208\184\45\209\129\208\187\208\176\208\177\208\190\63"

  bool testSymbols() {
    PREPARE_LEXER(LONG_SYMBOL1 " " LONG_SYMBOL2 " 12.23a q13 1..3. -.");

    bool passed =
      lex.readToken() == TOKEN_SYMBOL && lex.string() == LONG_SYMBOL1 &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == LONG_SYMBOL2 &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == "12.23" &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == "q13" &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == "1..3" &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == "-." &&
      lex.readToken() == TOKEN_EOF;

    return printTestResult(subj, "symbols", passed);
  }

  bool testStrLiterals() {
    PREPARE_LEXER("\"" LONG_SYMBOL2 "\"\"\\\"quote\\\"\" \"eof");

    bool passed =
      lex.readToken() == TOKEN_STR_LIT && lex.string() == LONG_SYMBOL1 &&
      lex.readToken() == TOKEN_STR_LIT && lex.string() == "\"quote\"" &&
      lex.readToken() == TOKEN_ERROR;
      
    return printTestResult(subj, "strLiterals", passed);
  }

  bool testPosInts() {
    PREPARE_LEXER("0 +0 -0 +2 18446744073709551615 18446744073709551616");

    bool passed =
      lex.readToken() == TOKEN_POS_INT && lex.posInt() == 0 &&
      lex.readToken() == TOKEN_POS_INT && lex.posInt() == 0 &&
      lex.readToken() == TOKEN_POS_INT && lex.posInt() == 0 &&
      lex.readToken() == TOKEN_POS_INT && lex.posInt() == 2 &&
      lex.readToken() == TOKEN_POS_INT &&
      lex.posInt() == 18446744073709551615ULL && 
      lex.readToken() == TOKEN_ERROR;

    return printTestResult(subj, "posInts", passed);
  }

  bool testNegInts() {
    PREPARE_LEXER("-1 -9223372036854775808 -9223372036854775809");

    bool passed =
      lex.readToken() == TOKEN_NEG_INT && lex.negInt() == -1 &&
      lex.readToken() == TOKEN_NEG_INT &&
      lex.negInt() == -9223372036854775807LL - 1LL && 
      lex.readToken() == TOKEN_ERROR;

    return printTestResult(subj, "negInts", passed);
  }

  const double EPSILON = .0000001;

#define DEQUAL(a, b) (abs(a - b) < EPSILON)

  bool testReals() {
    PREPARE_LEXER("0. -0. 0.1 -0.1 .0 .1 -.0 -2.1 1e2 .1E3 -.0e3 -1.e4 1e2.3");

    bool passed =
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), 0) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), 0) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), .1) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), -.1) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), 0) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), .1) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), 0) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), -2.1) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), 1e2) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), .1e3) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), -.0e3) &&
      lex.readToken() == TOKEN_REAL && DEQUAL(lex.real(), -1e4) &&
      lex.readToken() == TOKEN_SYMBOL && lex.string() == "1e2.3" &&
      lex.readToken() == TOKEN_EOF;

    return printTestResult(subj, "reals", passed);
  }

}

namespace Ant {
  namespace Lang {
    namespace Test {

      bool testLexer() {
        return
          testWhitespaces() &&
          testDelimiters() &&
          testSymbols() &&
          testStrLiterals() &&
          testPosInts() &&
          testNegInts() &&
          testReals();
      }

    }
  }
}
