#include "exception.h"
#include "string.h"
#include "utf8/unchecked.h"

namespace {

    inline void throwIfInvalid(const std::string &str) {
      if (utf8::find_invalid(str.begin(), str.end()) != str.end())
        throw Ant::EncodingException();
    }

}

namespace Ant {

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
    return out << str.c_str();
  }

}
