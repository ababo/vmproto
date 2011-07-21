#include "string.h"
#include "exception.h"
#include "../platform.h"
#include "utf8/source/utf8.h"

namespace Ant {
  namespace Common {

    using namespace std;
    using namespace utf8;

    ostream &operator<<(ostream &out, const String &str) {
      out << str.c_str();
    }

    wostream &operator<<(wostream &out, const String &str) {
      wstring wstr;
      wstr.reserve(str.length());

#ifndef SHORT_WCHAR
      utf8to32(str.s.begin(), str.s.end(), back_inserter(wstr));
#else
      utf8to16(str.s.begin(), str.s.end(), back_inserter(wstr));
#endif

      return out << wstr;
    }

#define THROW_INVALID(str) \
    if (find_invalid(str.begin(), str.end()) != str.end()) \
      throw EncodingException();

    String::String(const char *str) : s(str) {
      THROW_INVALID(s);
    }

    size_t String::length() const {
      return utf8::distance(s.begin(), s.end());
    }

  }
}
