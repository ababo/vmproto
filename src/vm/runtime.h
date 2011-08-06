#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <map>
#include <vector>

#include "module.h"
#include "../common/uuid.h"
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
      struct ModuleData {
        std::vector<VarTypeData> vtypes;
        std::vector<VarTypeId> refs;
        std::vector<VarTypeId> regs;
        std::vector<ProcData> procs;
        std::vector<VMCodeByte> code;
      };
      typedef std::map<Common::UUID, ModuleData> ModuleDataMap;

      const ModuleData *findModuleData(const Common::UUID &id) const {
        ModuleDataMap::const_iterator i = modules.find(id);
        return i != modules.end() ? &i->second : NULL;
      }
      void insertModuleData(const Common::UUID &id, const ModuleData &data) {
        modules.insert(ModuleDataMap::value_type(id, data));
      }
      void removeModuleData(const Common::UUID &id) {
        modules.erase(id);
      }

      ModuleDataMap modules;

    private:
      Runtime() : Common::Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
