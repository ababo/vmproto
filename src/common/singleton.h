#ifndef __COMMON_SINGLETON_INCLUDED__
#define __COMMON_SINGLETON_INCLUDED__

namespace Ant {
  namespace Common {

    template <class T>
    class Singleton {
    public:
      static T &instance() {
        static T inst;
        return inst;
      }

    protected:
      Singleton(int) {} // subclass must have a private constructor
    };

  }
}

#endif // __COMMON_SINGLETON_INCLUDED__
