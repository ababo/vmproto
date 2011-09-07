#ifndef __SINGLETON_INCLUDED__
#define __SINGLETON_INCLUDED__

namespace Ant {

  template<class T> class Singleton {
  public:
    static T &instance() {
      static T inst;
      return inst;
    }

  protected:
    Singleton(int) {} // subclass must have a private constructor
  };

}

#endif // __SINGLETON_INCLUDED__
