#include "../instr.h"
#include "../mbuilder.h"

namespace Ant {
  namespace VM {
    namespace Test {

      void createFactorialModule(Module &module) {
        ModuleBuilder builder;

        VarTypeId wordType = builder.addVarType(1, 8);

        RegId io = builder.addReg(wordType);
        RegId pr = builder.addReg(wordType);

        ProcId proc = builder.addProc(PFLAG_EXTERNAL | PFLAG_FUNCTION, io);
        builder.addProcInstr(proc, JNZInstr(io, 3));
        builder.addProcInstr(proc, MOVM8Instr(1, io));
        builder.addProcInstr(proc, RETInstr());
        builder.addProcInstr(proc, ASTInstr(pr));
        builder.addProcInstr(proc, MOVM8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr));
        builder.addProcInstr(proc, DECInstr(io));
        builder.addProcInstr(proc, JNZInstr(io, -2));
        builder.addProcInstr(proc, MOVN8Instr(pr, io));
        builder.addProcInstr(proc, FSTInstr());
        builder.addProcInstr(proc, RETInstr());

        builder.createModule(module);
      }

      void createQSortModule(Module &module) {

      }

    }
  }
}
