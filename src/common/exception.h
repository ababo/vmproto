#ifndef __COMMON_EXCEPTION_INCLUDED__
#define __COMMON_EXCEPTION_INCLUDED__

#include <exception>

namespace Ant {
  namespace Common {

#define EXCEPTION(name, parent, message) \
    class name : public parent { \
    public: \
      virtual const char *what() const throw() { \
        return message; } \
    };

    EXCEPTION(Exception, std::exception, "generic error");
    EXCEPTION(CodePointException, Exception, "invalid code point");
    EXCEPTION(EncodingException, Exception, "bad encoding");
    EXCEPTION(EndOfFileException, Exception, "unexpected end of file");
    EXCEPTION(OutOfRangeException, Exception, "value out of range");
    EXCEPTION(OperationException, Exception, "bad operation");
    EXCEPTION(EscapeCharException, Exception, "bad escape character");
    EXCEPTION(IOException, Exception, "input/output error");

  }
}

#endif // __COMMON_EXCEPTION_INCLUDED__
