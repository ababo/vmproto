#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <map>
#include <stdint.h>

#include "../singleton.h"
#include "../uuid.h"
#include "vmdefs.h"

namespace Ant {
  namespace VM {

    class Runtime : public Singleton<Runtime> {
      friend class Singleton<Runtime>;
      friend class ModuleBuilder;
      friend class Module;
    protected:
      struct ModuleData;
      typedef std::map<UUID, ModuleData*> ModuleDataMap;
      typedef ModuleDataMap::value_type ModuleDataPair;
      typedef ModuleDataMap::iterator ModuleDataIterator;

      ModuleDataIterator retainModuleData(const UUID &id);
      void releaseModuleData(ModuleDataIterator moduleDataIter);
      void insertModuleData(const UUID &id, ModuleData &moduleData);

      ModuleDataMap modules;

    private:
      Runtime() : Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
