#ifndef __VM_MODULE_INCLUDED__
#define __VM_MODULE_INCLUDED__

#include <stdint.h>

#include "../common/farray.h"

namespace Ant {
  namespace VM {

    typedef uint16_t VarTypeId;
    typedef uint16_t ProcId;
    typedef uint16_t RegId;

    typedef const uint8_t *VMCode;
    typedef const void *NativeCode;

    struct Variable {};

    template <int Bytes, int VRefs, int PRefs, int Count>
    struct StaticVariable : public Variable {
      struct {
        uint8_t bytes[Bytes];
        Variable *vrefs[VRefs];
        NativeCode prefs[PRefs];
      } elts[Count];
    };

    struct VarType {
      uint32_t count;
      uint32_t bytes;
      Common::FixedArray<VarTypeId> vrefs;
      Common::FixedArray<VarTypeId> prefs;
    };

    struct Proc {
      uint16_t flags;
      VarTypeId io;
      Common::FixedArray<uint8_t> code;
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
