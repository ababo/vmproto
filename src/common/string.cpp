#include "string.h"
#include "utf8/source/utf8.h"

namespace Ant {
  namespace Common {

    std::ostream &operator<<(std::ostream &out, const String &str) {
      out << str.c_str();
    }

    std::wostream &operator<<(std::wostream &out, const String &str) {
      
      return out;
    }

    String::String(const char *str) {


    }

    size_t String::length() const {

      return 0;
    }

  }
}
