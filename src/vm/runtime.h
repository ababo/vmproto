#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include "module.h"
#include "../common/farray.h"
#include "../common/singleton.h"

namespace Ant {
  namespace VM {

    class Runtime : public Common::Singleton<Runtime> {
      friend class Common::Singleton<Runtime>;
      friend class ModuleBuilder;
      friend class Module;
    protected:
      struct VarTypeData {
        size_t count;
        size_t bytes;
        Common::FixedArray<VarTypeId> vrefs;
        Common::FixedArray<VarTypeId> prefs;
      };
      struct ProcData {
        unsigned int flags;
        VarTypeId io;
        Common::FixedArray<VMCodeByte> code;
      };

    private:
      Runtime() : Common::Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
