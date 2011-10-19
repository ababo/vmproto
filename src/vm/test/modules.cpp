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
        RegId io = builder.addReg(0, vtype);
        ProcTypeId ptype = builder.addProcType(0, io);
        ProcId proc = builder.addProc(PFLAG_EXTERNAL, ptype);
        builder.addProcInstr(proc, JNZInstr(io, 3));
        builder.addProcInstr(proc, CPI8Instr(1, io));
        builder.addProcInstr(proc, RETInstr());
        RegId pr = builder.addReg(0, vtype);
        builder.addProcInstr(proc, PUSHInstr(pr));
        builder.addProcInstr(proc, CPI8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr));
        builder.addProcInstr(proc, DECInstr(io));
        builder.addProcInstr(proc, JNZInstr(io, -2));
        builder.addProcInstr(proc, CPBInstr(pr, io));
        builder.addProcInstr(proc, POPInstr());
        builder.addProcInstr(proc, RETInstr());

        builder.createModule(module);
      }

      void createQSortModule(Module &module) {
        ModuleBuilder builder;

        // struct ioType { int l, h, *a; };
        VarTypeId wordType = builder.addVarType(8);
        VarTypeId ioType = builder.addVarType(16);
        builder.addVarTypeVRef(ioType, VFLAG_NON_FIXED, wordType);

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
        RegId io = builder.addReg(0, ioType);
        ProcTypeId ptype = builder.addProcType(0, io);
        ProcId part = builder.addProc(0, ptype);
        RegId l = builder.addReg(0, wordType);
        builder.addProcInstr(part, PUSHInstr(l));
        builder.addProcInstr(part, CPBInstr(io, l));
        RegId h = builder.addReg(0, wordType);
        builder.addProcInstr(part, PUSHInstr(h));
        builder.addProcInstr(part, LDBInstr(io, 8, h));
        RegId a = builder.addReg(VFLAG_NON_FIXED, wordType);
        builder.addProcInstr(part, PUSHRInstr(a));
        builder.addProcInstr(part, LDRInstr(io, 0, a));
        RegId al = builder.addReg(0, wordType);
        builder.addProcInstr(part, PUSHInstr(al));
        RegId ah = builder.addReg(0, wordType);
        builder.addProcInstr(part, PUSHInstr(ah));
        builder.addProcInstr(part, LDEInstr(a, h, ah));
        builder.addProcInstr(part, LDEInstr(a, l, al));
        builder.addProcInstr(part, JGInstr(al, ah, 7));
        builder.addProcInstr(part, PUSHInstr(h));
        builder.addProcInstr(part, LDEInstr(a, io, h));
        builder.addProcInstr(part, STEInstr(al, a, io));
        builder.addProcInstr(part, STEInstr(h, a, l));
        builder.addProcInstr(part, INCInstr(io));
        builder.addProcInstr(part, POPInstr());
        builder.addProcInstr(part, INCInstr(l));
        builder.addProcInstr(part, JNGInstr(l, h, -9));
        builder.addProcInstr(part, DECInstr(io));
        builder.addProcInstr(part, POPLInstr(0));
        builder.addProcInstr(part, RETInstr());

        // void qsort(struct ioType *io) {
        //   int l = io->l, h = io->h;
        //   if(h > l)
        //     goto l1;
        //   return;
        // l1:
        //   {
        //     int *a = io->a;
        //     struct ioType cio;
        //     cio.l = l, cio.h = h, cio.a = a;
        //     part(&cio);
        //     h = cio.l++;
        //     qsort(&cio);
        //     cio.l = l, cio.h = --h;
        //     qsort(&cio);
        //   }
        // }
        ProcId qsort = builder.addProc(PFLAG_EXTERNAL, ptype);
        builder.addProcInstr(qsort, PUSHInstr(l));
        builder.addProcInstr(qsort, CPBInstr(io, l));
        builder.addProcInstr(qsort, PUSHInstr(h));
        builder.addProcInstr(qsort, LDBInstr(io, 8, h));
        builder.addProcInstr(qsort, JGInstr(h, l, 2));
        builder.addProcInstr(qsort, RETInstr());
        RegId cio = builder.addReg(0, ioType);
        builder.addProcInstr(qsort, PUSHRInstr(a));
        builder.addProcInstr(qsort, LDRInstr(io, 0, a));
        builder.addProcInstr(qsort, PUSHInstr(cio));
        builder.addProcInstr(qsort, CPBInstr(l, cio));
        builder.addProcInstr(qsort, STBInstr(h, cio, 8));
        builder.addProcInstr(qsort, STRInstr(a, cio, 0));
        builder.addProcInstr(qsort, CALLInstr(part));
        builder.addProcInstr(qsort, CPBInstr(cio, h));
        builder.addProcInstr(qsort, INCInstr(cio));
        builder.addProcInstr(qsort, CALLInstr(qsort));
        builder.addProcInstr(qsort, CPBInstr(l, cio));
        builder.addProcInstr(qsort, DECInstr(h));
        builder.addProcInstr(qsort, STBInstr(h, cio, 8));
        builder.addProcInstr(qsort, CALLInstr(qsort));
        builder.addProcInstr(qsort, POPLInstr(0));
        builder.addProcInstr(qsort, RETInstr());

        builder.createModule(module);
      }

      void createEHTestModule(Module &module) {
        ModuleBuilder builder;

        // int ed;
        // void func1(int *io) {
        //   if(*io == ed)
        //     goto l1;
        //   {
        //     int io;
        //     throw ed;
        //   }
        // l1:
        // }
        VarTypeId wordType = builder.addVarType(8);
        RegId ed = 0, io = builder.addReg(0, wordType);
        ProcTypeId ptype = builder.addProcType(0, io);
        ProcId func1 = builder.addProc(0, ptype);
        builder.addProcInstr(func1, JEInstr(io, ed, 4));
        builder.addProcInstr(func1, PUSHInstr(io));
        builder.addProcInstr(func1, THROWInstr());
        builder.addProcInstr(func1, POPInstr());
        builder.addProcInstr(func1, RETInstr());

        // void func2(int *io) {
        //  ed = 0;
        //  try {
        //    try {
        //      int cio = *io;
        //      func1(&cio);
        //      *io = -1;
        //    }
        //    catch(...) {
        //      int cio = *io;
        //      ed = *io; ed *= ed;
        //      func1(&cio);
        //      *io = -2;
        //    }
        //  }
        //  catch(...) {
        //    ed = -3;
        //    throw ed;
        //  }
        // }
        ProcId func2 = builder.addProc(PFLAG_EXTERNAL, ptype);
        builder.addProcInstr(func2, CPI8Instr(0, ed));
        builder.addProcInstr(func2, PUSHHInstr(16));
        builder.addProcInstr(func2, PUSHHInstr(7));
        RegId cio = builder.addReg(0, wordType);
        builder.addProcInstr(func2, PUSHInstr(cio));
        builder.addProcInstr(func2, CPBInstr(io, cio));
        builder.addProcInstr(func2, CALLInstr(func1));
        builder.addProcInstr(func2, CPI8Instr(uint64_t(-1), io));
        builder.addProcInstr(func2, POPInstr());
        builder.addProcInstr(func2, JMPInstr(7));
        builder.addProcInstr(func2, PUSHInstr(cio));
        builder.addProcInstr(func2, CPBInstr(io, cio));
        builder.addProcInstr(func2, CPBInstr(io, ed));
        builder.addProcInstr(func2, MULInstr(ed, ed, ed));
        builder.addProcInstr(func2, CALLInstr(func1));
        builder.addProcInstr(func2, CPI8Instr(uint64_t(-2), io));
        builder.addProcInstr(func2, POPLInstr(1));
        builder.addProcInstr(func2, JMPInstr(3));
        builder.addProcInstr(func2, CPI8Instr(uint64_t(-3), ed));
        builder.addProcInstr(func2, THROWInstr());
        builder.addProcInstr(func2, POPInstr());
        builder.addProcInstr(func2, RETInstr());

        builder.createModule(module);
      }

    }
  }
}
