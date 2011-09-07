#include <memory>

#include "mdata.h"

namespace Ant {
  namespace VM {

    using namespace std;

    Runtime::ModuleDataIterator Runtime::retainModuleData(const UUID &id) {
      ModuleDataIterator i = modules.find(id);

      if(i != modules.end() && !i->second->isDropped()) {
        i->second->retain();
        return i;
      }

      return modules.end();
    }

    void Runtime::releaseModuleData(ModuleDataIterator moduleDataIter) {
      ModuleData *data = moduleDataIter->second;

      data->release();
      if(data->retainCount() == 1 && data->isDropped()) {
        modules.erase(moduleDataIter);
        data->release();
      }
    }

    void Runtime::insertModuleData(const UUID &id, ModuleData &moduleData) {
      auto_ptr<ModuleData> ptr(new ModuleData(id));
      ModuleDataPair p(id, ptr.get());
      pair<ModuleDataIterator, bool> ip = modules.insert(p);

      if(ip.second)
        ptr.release();

      ip.first->second->take(moduleData);
    }

  }
}
