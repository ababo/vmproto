#ifndef __VM_MODULE_INCLUDED__
#define __VM_MODULE_INCLUDED__

#include <stdint.h>

namespace Ant {
  namespace VM {

    typedef unsigned int VarTypeId;
    typedef unsigned int ProcId;
    typedef unsigned int RegId;

    typedef const uint8_t *VMCode;
    typedef const void *NativeCode;

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FUNCTION = 0x2
    };

    class Module {

    };

  }
}

#endif // __VM_MODULE_INCLUDED__
