#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <map>
#include <vector>

#include "../common/farray.h"
#include "../common/singleton.h"
#include "../common/uuid.h"

namespace Ant {
  namespace VM {

    typedef unsigned int VarTypeId;
    typedef unsigned int ProcId;
    typedef unsigned int RegId;

    typedef unsigned char VMCodeByte;
    typedef const VMCodeByte *VMCode;
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
        ModuleData &operator=(ModuleData& moduleData) {
          vtypes.swap(moduleData.vtypes);
          refs.swap(moduleData.refs);
          regs.swap(moduleData.regs);
          procs.swap(moduleData.procs);
          code.swap(moduleData.code);
        }

        std::vector<VarTypeData> vtypes;
        std::vector<VarTypeId> refs;
        std::vector<VarTypeId> regs;
        std::vector<ProcData> procs;
        std::vector<VMCodeByte> code;
      };
      typedef std::map<Common::UUID, ModuleData> ModuleDataMap;
      typedef ModuleDataMap::value_type ModuleDataPair;
      typedef ModuleDataMap::const_iterator ModuleDataConstIterator;
      typedef ModuleDataMap::iterator ModuleDataIterator;

      const ModuleData *findModuleData(const Common::UUID &id) const {
        ModuleDataConstIterator i = modules.find(id);
        return i != modules.end() ? &i->second : NULL;
      }
      void insertModuleData(const Common::UUID &id, ModuleData &moduleData) {
        ModuleDataPair p = ModuleDataPair(id, ModuleData());
        ModuleDataIterator i = modules.insert(p).first;
        i->second = moduleData;
      }
      void dropModuleData(const Common::UUID &id) {
        modules.erase(id);
      }

      ModuleDataMap modules;

    private:
      Runtime() : Common::Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
