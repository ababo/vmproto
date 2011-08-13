#ifndef __VM_MODULE_INCLUDED__
#define __VM_MODULE_INCLUDED__

#include <cstddef>

#include "../common/farray.h"

namespace Ant {
  namespace VM {

    typedef unsigned int VarTypeId;
    typedef unsigned int ProcId;
    typedef unsigned int RegId;

    typedef const unsigned char *VMCode;
    typedef const void *NativeCode;

    struct Variable {};

    template <size_t Count, size_t Bytes, size_t VRefs, size_t PRefs>
    struct StaticVariable : public Variable {
      struct {
        unsigned char bytes[Bytes];
        Variable *vrefs[VRefs];
        NativeCode prefs[PRefs];
      } elts[Count];
    };

    struct VarType {
      size_t count;
      size_t bytes;
      Common::FixedArray<VarTypeId> vrefs;
      Common::FixedArray<VarTypeId> prefs;
    };

    struct Proc {
      unsigned int flags;
      VarTypeId io;
      Common::FixedArray<unsigned char> code;
    };

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FUNCTION = 0x2,
      PFLAG_FIRST_RESERVED = 0x4
    };

    class Module {

    };

  }
}

#endif // __VM_MODULE_INCLUDED__
