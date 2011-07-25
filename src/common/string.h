#ifndef __COMMON_STRING_INCLUDED__
#define __COMMON_STRING_INCLUDED__

#include <string>
#include <ostream>

#include "char.h"

namespace Ant {
  namespace Common {

    class String {
      friend std::ostream &operator<<(std::ostream &out, const String &str);
      friend std::wostream &operator<<(std::wostream &out, const String &str);
    public:
      class Iterator : public std::iterator <std::bidirectional_iterator_tag,
                                             Char> {
      public:
        Iterator(const String &str, bool end)
          : s(str.s) { i = end ? s.size() : 0; }

        Char operator*() const;
        Iterator &operator++();
        Iterator &operator--();
        Iterator &operator++(int);
        Iterator &operator--(int);

      protected:
        const std::string &s;
        int i;
      };

      String() {}
      String(const char *str);

      operator const std::string&() const { return s; }
      bool operator==(const std::string str) const { return str == s; }

      const char *c_str() const { return s.c_str(); }
      size_t size() const { return s.size(); }
      size_t length() const;

      Iterator begin() const { return Iterator(*this, false); }
      Iterator end() const { return Iterator(*this, true); }

      void push_back(Char chr);

    protected:
      std::string s;
    };

  }
}

#endif // __COMMON_STRING_INCLUDED__
