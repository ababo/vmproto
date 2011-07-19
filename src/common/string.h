#ifndef __COMMON_STRING_INCLUDED__
#define __COMMON_STRING_INCLUDED__

#include <string>
#include <ostream>

namespace Ant {
  namespace Common {

    class String : public std::string {
      friend std::ostream &operator<<(std::ostream &output, const String &s);
    public:
      String() {}
      String(const char *s) : std::string(s) {}
    };

  }
}

#endif // __COMMON_STRING_INCLUDED__
