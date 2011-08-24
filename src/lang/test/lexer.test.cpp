#include <sstream>
#include <cmath>

#include "../../exception.h"
#include "../../test/test.h"
#include "../lexer.h"

namespace {

  using namespace std;
  using namespace Ant;
  using namespace Ant::Test;
  using namespace Ant::Lang;

  const String subj = "Ant::Lang::Lexer";

#define PREPARE_LEXER(buf_content) \
  const String buf = buf_content; \
  istringstream in(buf); \
  Location loc; \
  Lexer lex(in, loc); \
  bool passed;

  bool testWhitespaces() {
    PREPARE_LEXER("; comment \n \r\t\n\r\n");

    try { 
      passed = lex.readToken() == TOKEN_EOF;
      passed = passed && lex.location() == Location(4, 1);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "whitespaces", passed);
  }

  bool testDelimiters() {
    PREPARE_LEXER(".)).. .(.");

    try {
      passed = lex.readToken() == TOKEN_DOT;
      passed = passed && lex.readToken() == TOKEN_CLOSE;
      passed = passed && lex.readToken() == TOKEN_CLOSE;
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == "..";
      passed = passed && lex.readToken() == TOKEN_DOT;
      passed = passed && lex.readToken() == TOKEN_OPEN;
      passed = passed && lex.readToken() == TOKEN_DOT;
      passed = passed && lex.readToken() == TOKEN_EOF;
      passed = passed && lex.location() == Location(1, 10);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "delimiters", passed);
  }

#define LONG_SYMBOL1 "H!e@l#l$o%_^w&o*r~l<d>?...+-/"
#define LONG_SYMBOL2 "\xD0\x9D\xD0\xB0\x2D\xD1\x80" \
  "\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xBE\xD0\xBC"

  bool testStrings() {
    PREPARE_LEXER("\"" LONG_SYMBOL2 "\"\"\\\"quote\\\"\" \"eof");

    try {
      passed = lex.readToken() == TOKEN_STRING;
      passed = passed && lex.string() == LONG_SYMBOL2;
      passed = passed && lex.readToken() == TOKEN_STRING;
      passed = passed && lex.string() == "\"quote\"";
      passed = passed && lex.location() == Location(1, 24);
    }
    catch(...) { passed = false; }

    ASSERT_THROW(lex.readToken(), EndOfFileException);
 
    passed = passed && lex.location() == Location(1, 29);

    return printTestResult(subj, "strings", passed);
  }

  bool testPosInts() {
    PREPARE_LEXER("0 +0 -0 +2 18446744073709551615 18446744073709551616");

    try {
      passed = lex.readToken() == TOKEN_POS_INT;
      passed = passed && lex.posInt() == 0;
      passed = passed && lex.readToken() == TOKEN_POS_INT;
      passed = passed && lex.posInt() == 0;
      passed = passed && lex.readToken() == TOKEN_POS_INT;
      passed = passed && lex.posInt() == 0;
      passed = passed && lex.readToken() == TOKEN_POS_INT;
      passed = passed && lex.posInt() == 2;
      passed = passed && lex.readToken() == TOKEN_POS_INT;
      passed = passed && lex.posInt() == 18446744073709551615ULL;
      passed = passed && lex.location() == Location(1, 32);
    }
    catch(...) { passed = false; }

    ASSERT_THROW(lex.readToken(), RangeException);

    passed = passed && lex.location() == Location(1, 33);

    return printTestResult(subj, "posInts", passed);
  }

  bool testNegInts() {
    PREPARE_LEXER("-1 -9223372036854775808 -9223372036854775809");

    try {
      passed = lex.readToken() == TOKEN_NEG_INT;
      passed = passed && lex.negInt() == -1;
      passed = passed && lex.readToken() == TOKEN_NEG_INT;
      passed = passed && lex.negInt() == -9223372036854775807LL - 1LL;
      passed = passed && lex.location() == Location(1, 24);
    }
    catch(...) { passed = false; }

    ASSERT_THROW(lex.readToken(), RangeException);

    passed = passed && lex.location() == Location(1, 25);

    return printTestResult(subj, "negInts", passed);
  }

  inline bool dequal(double a, double b) {
    return abs(a - b) < .0000001;
  }

  bool testReals() {
    PREPARE_LEXER("0. -0. 0.1 -0.1 .0 .1 -.0 -2.1 1e2 .1E3 -.0e3 -1.e4 1e2.3");

    try {
      passed = lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), 0);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), 0);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), .1);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), -.1);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), 0);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), .1);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), 0);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), -2.1);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), 1e2);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), .1e3);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), -.0e3);
      passed = passed && lex.readToken() == TOKEN_REAL;
      passed = passed && dequal(lex.real(), -1e4);
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == "1e2.3";
      passed = passed && lex.readToken() == TOKEN_EOF;
      passed = passed && lex.location() == Location(1, 58);
    }
    catch(...) { passed = false; }
    
    return printTestResult(subj, "reals", passed);
  }

  bool testSymbols() {
    PREPARE_LEXER(LONG_SYMBOL1 " " LONG_SYMBOL2 " 12.23a q13 1..3. -.");

    try {
      passed = lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == LONG_SYMBOL1;
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == LONG_SYMBOL2;
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == "12.23a";
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == "q13";
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == "1..3.";
      passed = passed && lex.readToken() == TOKEN_SYMBOL;
      passed = passed && lex.string() == "-.";
      passed = passed && lex.readToken() == TOKEN_EOF;
      passed = passed && lex.location() == Location(1, 61);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "symbols", passed);
  }

}

namespace Ant {
  namespace Lang {
    namespace Test {

      bool testLexer() {
        bool passed;

        passed = testWhitespaces();
        passed = passed && testDelimiters();
        passed = passed && testStrings();
        passed = passed && testPosInts();
        passed = passed && testNegInts();
        passed = passed && testReals();
        passed = passed && testSymbols();

        return passed;
      }

    }
  }
}
