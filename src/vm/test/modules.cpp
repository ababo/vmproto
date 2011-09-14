#include "../instr.h"
#include "../mbuilder.h"

namespace Ant {
  namespace VM {
    namespace Test {

      void createFactorialModule(Module &module) {
        ModuleBuilder builder;

        // void fact(unsigned int *io) {
        //   if(*io)
        //     goto l1;
        //   *io = 1;
        //   return;
        // l1:
        //   {
        //     unsigned int pr = 1;
        //   l2:
        //     pr = *io * pr;
        //     if(--*io)
        //       goto l2;
        //     *io = pr;
        //   }
        // }
        VarTypeId vtype = builder.addVarType(8);
        RegId io = builder.addReg(vtype);
        ProcTypeId ptype = builder.addProcType(0, io);
        ProcId proc = builder.addProc(PFLAG_EXTERNAL, ptype);
        builder.addProcInstr(proc, JNZInstr(io, 3)); //--|
        builder.addProcInstr(proc, CPI8Instr(1, io)); // |
        builder.addProcInstr(proc, RETInstr()); //       |
        RegId pr = builder.addReg(vtype); //             |
        builder.addProcInstr(proc, ALSInstr(pr)); //<----|
        builder.addProcInstr(proc, CPI8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr)); //<-|
        builder.addProcInstr(proc, DECInstr(io)); //          |
        builder.addProcInstr(proc, JNZInstr(io, -2)); //------|
        builder.addProcInstr(proc, CPBInstr(pr, io));
        builder.addProcInstr(proc, FRSInstr());
        builder.addProcInstr(proc, RETInstr());

        builder.createModule(module);
      }

      void createQSortModule(Module &module) {
        ModuleBuilder builder;

        // struct ioType { int l, h, *a; };
        VarTypeId wordType = builder.addVarType(8);
        VarTypeId ioType = builder.addVarType(16);
        builder.addVarTypeVRef(ioType, wordType, 0);

        // void part(struct ioType *io) {
        //   int l = io->l, h = io->h, *a = io->a, al, ah = a[h];
        // l1:
        //   al = a[l];
        //   if(al > ah)
        //     goto l2;
        //   { int h = a[io->l]; a[io->l] = al, a[l] = h, ++io->l; }
        // l2:
        //   if(!(++l > h))
        //     goto l1;
        //   --io->l;
        // };
        RegId io = builder.addReg(ioType);
        ProcTypeId ptype = builder.addProcType(0, io);
        ProcId part = builder.addProc(0, ptype);
        RegId l = builder.addReg(wordType);
        builder.addProcInstr(part, ALSInstr(l));
        builder.addProcInstr(part, CPBInstr(io, l));
        RegId h = builder.addReg(wordType);
        builder.addProcInstr(part, ALSInstr(h));
        builder.addProcInstr(part, LDBInstr(io, 8, h));
        RegId a = builder.addReg(wordType, 0);
        builder.addProcInstr(part, ALSRInstr(a));
        builder.addProcInstr(part, LDRInstr(io, 0, a));
        RegId al = builder.addReg(wordType);
        builder.addProcInstr(part, ALSInstr(al));
        RegId ah = builder.addReg(wordType);
        builder.addProcInstr(part, ALSInstr(ah));
        builder.addProcInstr(part, LDEInstr(a, h, ah));
        builder.addProcInstr(part, LDEInstr(a, l, al)); // <--|
        builder.addProcInstr(part, JGInstr(al, ah, 7)); // -| |
        builder.addProcInstr(part, ALSInstr(h)); //         | |
        builder.addProcInstr(part, LDEInstr(a, io, h)); //  | |
        builder.addProcInstr(part, STEInstr(al, a, io)); // | |
        builder.addProcInstr(part, STEInstr(h, a, l)); //   | |
        builder.addProcInstr(part, INCInstr(io)); //        | |
        builder.addProcInstr(part, FRSInstr()); //          | |
        builder.addProcInstr(part, INCInstr(l)); // <-------| |
        builder.addProcInstr(part, JNGInstr(l, h, -9)); // ---|
        builder.addProcInstr(part, DECInstr(io));
        builder.addProcInstr(part, FRSNInstr(5));
        builder.addProcInstr(part, RETInstr());

        // void qsort(struct ioType *io) {
        //   int l = io->l, h = io->h;
        //   if(h > l)
        //     goto l1;
        //   return;
        // l1:
        //   part(io);
        //   h = io->l++;
        //   qsort(io);
        //   io->l = l, io->h = --h;
        //   qsort(io);
        // }
        ProcId qsort = builder.addProc(PFLAG_EXTERNAL, ptype);
        builder.addProcInstr(qsort, ALSInstr(l));
        builder.addProcInstr(qsort, CPBInstr(io, l));
        builder.addProcInstr(qsort, ALSInstr(h));
        builder.addProcInstr(qsort, LDBInstr(io, 8, h));
        builder.addProcInstr(qsort, JGInstr(h, l, 2)); // -|
        builder.addProcInstr(qsort, RETInstr()); //        |
        builder.addProcInstr(qsort, CALLInstr(part)); // <-|
        builder.addProcInstr(qsort, CPBInstr(io, h));
        builder.addProcInstr(qsort, INCInstr(io));
        builder.addProcInstr(qsort, CALLInstr(qsort));
        builder.addProcInstr(qsort, CPBInstr(l, io));
        builder.addProcInstr(qsort, DECInstr(h));
        builder.addProcInstr(qsort, STBInstr(h, io, 8));
        builder.addProcInstr(qsort, CALLInstr(qsort));
        builder.addProcInstr(part, FRSNInstr(2));
        builder.addProcInstr(qsort, RETInstr());
      }

    }
  }
}
