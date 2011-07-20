#ifndef __COMMON_STRING_INCLUDED__
#define __COMMON_STRING_INCLUDED__

#include <string>
#include <ostream>

namespace Ant {
  namespace Common {

    class String {
      friend std::ostream &operator<<(std::ostream &out, const String &str);
      friend std::wostream &operator<<(std::wostream &out, const String &str);
    public:
      String() {}
      String(const char *str);

      operator const std::string& () const { return s; }
      bool operator == (const std::string str) const { return str == s; }

      const char *c_str() const { return s.c_str(); }
      size_t size () const { return s.size(); }
      size_t length() const;

    protected:
      std::string s;
    };

  }
}

#endif // __COMMON_STRING_INCLUDED__
