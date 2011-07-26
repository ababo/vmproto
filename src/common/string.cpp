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
      if (i < 0 || i >= str.size())
        throw OperationException();
      
      return Char(utf8::unchecked::peek_next(&str[i]));
    }

    String::String(const char *str) : str(str) {
      throwIfInvalid(this->str);
    }

    size_t String::length() const {
      return utf8::unchecked::distance(str.begin(), str.end());
    }

    void String::push_back(Char chr) {
      if(!chr.isValid())
        throw CodePointException();

      utf8::unchecked::append(chr, std::back_inserter(str));
    }

    std::ostream &operator<<(std::ostream &out, const String &str) {
      out << str.c_str();
    }

    std::wostream &operator<<(std::wostream &out, const String &str) {
      std::wstring wstr;
      wstr.reserve(str.length());

#ifndef SHORT_WCHAR
      utf8::unchecked::utf8to32(str.str.begin(),
                                str.str.end(),
                                back_inserter(wstr));
#else
      utf8::unchecked::utf8to16(str.str.begin(),
                                str.str.end(),
                                back_inserter(wstr));
#endif

      return out << wstr;
    }

  }
}
