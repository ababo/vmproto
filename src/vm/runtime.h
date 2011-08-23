#ifndef __VM_RUNTIME_INCLUDED__
#define __VM_RUNTIME_INCLUDED__

#include <map>
#include <vector>

#include "../farray.h"
#include "../retained.h"
#include "../singleton.h"
#include "../uuid.h"

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

    struct VarType {
      size_t count;
      size_t bytes;
      std::vector<VarTypeId> vrefs;
      std::vector<VarTypeId> prefs;
    };

    struct Proc {
      unsigned int flags;
      VarTypeId io;
      std::vector<VMCodeByte> code;
    };

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FUNCTION = 0x2,
      PFLAG_FIRST_RESERVED = 0x4
    };

    class Runtime : public Singleton<Runtime> {
      friend class Singleton<Runtime>;
      friend class ModuleBuilder;
      friend class Module;
    protected:
      struct VarTypeData {
        size_t count;
        size_t bytes;
        FixedArray<VarTypeId> vrefs;
        FixedArray<VarTypeId> prefs;
      };
      struct ProcData {
        unsigned int flags;
        VarTypeId io;
        FixedArray<VMCodeByte> code;
      };
      struct ModuleData : Retained<ModuleData> {
        ModuleData() : dropped(false) {}

        unsigned int varTypeCount() const;
        unsigned int regCount() const;
        unsigned int procCount() const;

        void varTypeById(VarTypeId id, VarType &vtype) const;
        VarTypeId regTypeById(RegId id) const;
        void procById(ProcId id, Proc &proc) const;

        void take(ModuleData& moduleData);

        void assertNotDropped() const;

        bool dropped;
        std::vector<VarTypeData> vtypes;
        std::vector<VarTypeId> refs;
        std::vector<VarTypeId> regs;
        std::vector<ProcData> procs;
        std::vector<VMCodeByte> code;
      };
      typedef std::map<UUID, ModuleData> ModuleDataMap;
      typedef ModuleDataMap::value_type ModuleDataPair;
      typedef ModuleDataMap::iterator ModuleDataIterator;

      ModuleDataIterator retainModuleData(const UUID &id);
      void releaseModuleData(ModuleDataIterator moduleDataIter);
      void insertModuleData(const UUID &id, ModuleData &moduleData);
      void dropModuleData(ModuleDataIterator moduleDataIter);

      ModuleDataMap modules;

    private:
      Runtime() : Singleton<Runtime>(0) {}
    };

  }
}

#endif // __VM_RUNTIME_INCLUDED__
