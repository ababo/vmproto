#include "string.h"

namespace Ant {
  namespace Common {

    std::ostream &operator<<(std::ostream &output, const String &s) {
      output << s.c_str();
    }

  }
}
