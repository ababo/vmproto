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
        builder.addProcInstr(proc, CPI8Instr(1, io));
        builder.addProcInstr(proc, RETInstr());
        builder.addProcInstr(proc, ALSInstr(pr));
        builder.addProcInstr(proc, CPI8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr));
        builder.addProcInstr(proc, DECInstr(io));
        builder.addProcInstr(proc, JNZInstr(io, -2));
        builder.addProcInstr(proc, CPBInstr(pr, io));
        builder.addProcInstr(proc, FRSInstr());
        builder.addProcInstr(proc, RETInstr());

        builder.createModule(module);
      }

      void createQSortModule(Module &module) {
        ModuleBuilder builder;

        VarTypeId wordType = builder.addVarType(8);
        VarTypeId ioType = builder.addVarType(16);
        builder.addVarTypeVRef(ioType, wordType, 0);

        RegId io = builder.addReg(ioType);
        RegId m = builder.addReg(wordType, 0);
        RegId b = builder.addReg(wordType);
        RegId j = builder.addReg(wordType);
        RegId mb = builder.addReg(wordType);
        RegId mj = builder.addReg(wordType);
        RegId ma = builder.addReg(wordType);

        ProcTypeId ptype = builder.addProcType(0, io);

        ProcId part = builder.addProc(0, ptype);
        builder.addProcInstr(part, ALSRInstr(m));
        builder.addProcInstr(part, ALSInstr(b));
        builder.addProcInstr(part, ALSInstr(j));
        builder.addProcInstr(part, ALSInstr(mb));
        builder.addProcInstr(part, ALSInstr(t1));
        builder.addProcInstr(part, ALSInstr(t2));
        builder.addProcInstr(part, LDRInstr(io, 0, m));
        builder.addProcInstr(part, LDBInstr(io, 8, b));
        builder.addProcInstr(part, CPBInstr(io, j));
        builder.addProcInstr(part, LDEInstr(m, b, mb));
        builder.addProcInstr(part, LDEInstr(m, j, mj));// <----
        builder.addProcInstr(part, JGInstr(mj, mb, 5)); //--- |
        builder.addProcInstr(part, LDEInstr(m, io, ma)); // | |
        builder.addProcInstr(part, STEInstr(mj, m, io)); // | |
        builder.addProcInstr(part, STEInstr(ma, m, j)); //  | |
        builder.addProcInstr(part, INCInstr(io)); //        | |
        builder.addProcInstr(part, INCInstr(j)); //<--------- |
        builder.addProcInstr(part, JNGInstr(j, b, -8)); //-----
        builder.addProcInstr(part, FRSNInstr(6));
        builder.addProcInstr(part, DECInstr(io));
        builder.addProcInstr(part, RETInstr());

        ProcId qsort = builder.addProc(PFLAG_EXTERNAL, ptype);

      }

    }
  }
}
