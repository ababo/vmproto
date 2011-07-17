#ifndef __COMMON_RETAINED_INCLUDED__
#define __COMMON_RETAINED_INCLUDED__

namespace Ant {
  namespace Common {
    
    template <class T>
      class Retained {
    public:
      Retained() : count(1) {}
      
      unsigned int retainCount() { return count; }
      T *retain() { ++count; return static_cast<T*>(this); }
      void release() { if(!--count) delete this; }
      
    protected:
      virtual ~Retained() {}
      
      unsigned int count;
    };
    
  }
}

#endif // __COMMON_RETAINED_INCLUDED__
