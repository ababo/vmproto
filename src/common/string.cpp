#include "string.h"
#include "exception.h"
#include "../platform.h"
#include "utf8/source/utf8.h"

namespace {

    inline void throwIfInvalid(const std::string &str) {
      if (utf8::find_invalid(str.begin(), str.end()) != str.end())
        throw Ant::Common::EncodingException();
    }

}

namespace Ant {
  namespace Common {

    Char String::Iterator::operator*() const {
      if (i < 0 || i >= s.size())
        throw OperationException();
      
      uint32_t chr = utf8::unchecked::peek_next(&s[i]);
      return *reinterpret_cast<Char*>(&chr);
    }

    String::String(const char *str) : s(str) {
      throwIfInvalid(s);
    }

    size_t String::length() const {
      return utf8::distance(s.begin(), s.end());
    }

    void String::push_back(Char chr) {
      utf8::append(chr, std::back_inserter(s));
    }

    std::ostream &operator<<(std::ostream &out, const String &str) {
      out << str.c_str();
    }

    std::wostream &operator<<(std::wostream &out, const String &str) {
      std::wstring wstr;
      wstr.reserve(str.length());

#ifndef SHORT_WCHAR
      utf8::utf8to32(str.s.begin(), str.s.end(), back_inserter(wstr));
#else
      utf8::utf8to16(str.s.begin(), str.s.end(), back_inserter(wstr));
#endif

      return out << wstr;
    }

  }
}
