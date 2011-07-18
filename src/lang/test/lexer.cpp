#include <iostream>
#include <sstream>

#include "../lexer.h"

using namespace std;
using namespace Ant::Lang;

namespace {

  const char *test_category = "Ant::Lang::Lexer test: ";
  const char *passed = "passed";
  const char *failed = "failed";

  bool testWhitespaces() {
    const char *buf = "; comment \n \r\t\n\r\n";
    stringstream in(buf);
    Location loc = { 0, 0 };
    Lexer lex(in, loc);
    
    cout << test_category << "whitespaces... ";
    bool ok = lex.readToken() == TOKEN_EOF;
    cout << (ok ? passed : failed) << endl;
    return ok;
  }

  const char *space_test_buf = "; comment \n \r\t\n\r\n";
  const char *delim_test_buf = "p))i(s";
  const char *symbol_test_buf = "H!e@l#l$o%_^w&o*r~l<d>?...+-/ "
    "\65\45\208\191\208\190\45\209\128\209\131\209\129\209\129\208"
    "\186\208\184\45\209\129\208\187\208\176\208\177\208\190\63 "
    "112.23a q123 1..23. ..  -.. .";
  const char *str_test_buf = "\"Hello world!\" \"\"\"\" \"eof!";
  const char *pint_test_buf = "0 +0 -0 1 +2 18446744073709551615";
  const char *nint_test_buf = "-1 -9223372036854775808";
  const char *real_test_buf = "18446744073709551616 -9223372036854775809 "
    "0.0 -0.0 0. .0 -.0 232323212. -2453534. +3. .123 -.234 +.000012 "
    "0.00000000000000000023 23432762374237642387462837462387648.62348";
}

namespace Ant {
  namespace Lang {
    namespace Test {

      bool testLexer() {
        cout << test_category << "begins" << endl;
        
        return testWhitespaces();
      }

    }
  }
}
