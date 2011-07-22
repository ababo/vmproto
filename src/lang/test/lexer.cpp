#include <sstream>
#include <cmath>

#include "../../common/exception.h"
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
  Lexer lex(in, loc); \
  bool passed;

  bool testWhitespaces() {
    PREPARE_LEXER("; comment \n \r\t\n\r\n");

    try { passed = lex.readToken() == TOKEN_EOF; }
    catch(...) { passed = false; }

    return printTestResult(subj, "whitespaces", passed);
  }

  bool testDelimiters() {
    PREPARE_LEXER(".)).. .(.");

    try {
      passed =
        lex.readToken() == TOKEN_DOT &&
        lex.readToken() == TOKEN_CLOSE &&
        lex.readToken() == TOKEN_CLOSE &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == ".." &&
        lex.readToken() == TOKEN_DOT &&
        lex.readToken() == TOKEN_OPEN &&
        lex.readToken() == TOKEN_DOT &&
        lex.readToken() == TOKEN_EOF;
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "delimiters", passed);
  }

#define LONG_SYMBOL1 "H!e@l#l$o%_^w&o*r~l<d>?...+-/"
#define LONG_SYMBOL2 "\xD0\x9D\xD0\xB0\x2D\xD1\x80" \
  "\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xBE\xD0\xBC"

  bool testSymbols() {
    PREPARE_LEXER(LONG_SYMBOL1 " " LONG_SYMBOL2 " 12.23a q13 1..3. -.");

    try {
      passed =
        lex.readToken() == TOKEN_SYMBOL && lex.string() == LONG_SYMBOL1 &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == LONG_SYMBOL2 &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == "12.23" &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == "q13" &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == "1..3" &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == "-." &&
        lex.readToken() == TOKEN_EOF;
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "symbols", passed);
  }

  bool testStrLiterals() {
    PREPARE_LEXER("\"" LONG_SYMBOL2 "\"\"\\\"quote\\\"\" \"eof");

    try {
      passed =
        lex.readToken() == TOKEN_STR_LIT && lex.string() == LONG_SYMBOL1 &&
        lex.readToken() == TOKEN_STR_LIT && lex.string() == "\"quote\"";
    }
    catch(...) { passed = false; }

    if(passed) {
      passed = false;

      try { lex.readToken(); }
      catch(const EndOfFileException&) { passed = true; }
    }    
 
    return printTestResult(subj, "strLiterals", passed);
  }

  bool testPosInts() {
    PREPARE_LEXER("0 +0 -0 +2 18446744073709551615 18446744073709551616");

    try {
      passed =
        lex.readToken() == TOKEN_POS_INT && lex.posInt() == 0 &&
        lex.readToken() == TOKEN_POS_INT && lex.posInt() == 0 &&
        lex.readToken() == TOKEN_POS_INT && lex.posInt() == 0 &&
        lex.readToken() == TOKEN_POS_INT && lex.posInt() == 2 &&
        lex.readToken() == TOKEN_POS_INT &&
        lex.posInt() == 18446744073709551615ULL;
    }
    catch(...) { passed = false; }

    if(passed) {
      passed = false;

      try { lex.readToken(); }
      catch(const OutOfRangeException&) { passed = true; }
    }

    return printTestResult(subj, "posInts", passed);
  }

  bool testNegInts() {
    PREPARE_LEXER("-1 -9223372036854775808 -9223372036854775809");

    try {
      passed =
        lex.readToken() == TOKEN_NEG_INT && lex.negInt() == -1 &&
        lex.readToken() == TOKEN_NEG_INT &&
        lex.negInt() == -9223372036854775807LL - 1LL;
    }
    catch(...) { passed = false; }
    
    if(passed) {
      passed = false;

      try { lex.readToken(); }
      catch(const OutOfRangeException&) { passed = true; }
    }


    return printTestResult(subj, "negInts", passed);
  }

  inline bool dequal(double a, double b) {
    return abs(a - b) < .0000001;
  }

  bool testReals() {
    PREPARE_LEXER("0. -0. 0.1 -0.1 .0 .1 -.0 -2.1 1e2 .1E3 -.0e3 -1.e4 1e2.3");

    try {
      passed =
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), 0) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), 0) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), .1) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), -.1) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), 0) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), .1) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), 0) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), -2.1) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), 1e2) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), .1e3) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), -.0e3) &&
        lex.readToken() == TOKEN_REAL && dequal(lex.real(), -1e4) &&
        lex.readToken() == TOKEN_SYMBOL && lex.string() == "1e2.3" &&
        lex.readToken() == TOKEN_EOF;
    }
    catch(...) { passed = false; }
    
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
