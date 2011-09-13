#include "../instr.h"
#include "../mbuilder.h"

namespace Ant {
  namespace VM {
    namespace Test {

      void createFactorialModule(Module &module) {
        ModuleBuilder builder;

        VarTypeId vtype = builder.addVarType(8);

        RegId io = builder.addReg(vtype);
        RegId pr = builder.addReg(vtype);

        ProcTypeId ptype = builder.addProcType(0, io);

        ProcId proc = builder.addProc(PFLAG_EXTERNAL, ptype);
        builder.addProcInstr(proc, JNZInstr(io, 3));
        builder.addProcInstr(proc, IMM8Instr(1, io));
        builder.addProcInstr(proc, RETInstr());
        builder.addProcInstr(proc, ASTInstr(pr));
        builder.addProcInstr(proc, IMM8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr));
        builder.addProcInstr(proc, DECInstr(io));
        builder.addProcInstr(proc, JNZInstr(io, -2));
        builder.addProcInstr(proc, CPBInstr(pr, io));
        builder.addProcInstr(proc, FSTInstr());
        builder.addProcInstr(proc, RETInstr());

        builder.createModule(module);
      }

      void createQSortModule(Module &module) {

      }

    }
  }
}
