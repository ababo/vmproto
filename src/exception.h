#ifndef __EXCEPTION_INCLUDED__
#define __EXCEPTION_INCLUDED__

#include <exception>

namespace Ant {

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
  EXCEPTION(RangeException, Exception, "value out of range");
  EXCEPTION(OperationException, Exception, "bad operation");
  EXCEPTION(EscapeCharException, Exception, "bad escape character");
  EXCEPTION(IOException, Exception, "input/output error");
  EXCEPTION(FlagsException, Exception, "invalid flags combination");
  EXCEPTION(ArgumentException, Exception, "bad call argument");
  EXCEPTION(NotFoundException, Exception, "object not found");
  EXCEPTION(TypeException, Exception, "invalid object type");
  EXCEPTION(BugException, Exception, "internal bug");
  EXCEPTION(EnvironmentException, Exception, "bad environment");
  EXCEPTION(RuntimeException, Exception, "runtime error");

}

#endif // __EXCEPTION_INCLUDED__
