#include "char.h"
#include "exception.h"
#include "utf8/source/utf8.h"

namespace Ant {
  namespace Common {

    using namespace std;
    using namespace utf8;

    istream &operator>>(istream &in, Char &chr) {

      return in;
    }

  }
}
