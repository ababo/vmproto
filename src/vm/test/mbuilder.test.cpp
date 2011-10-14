#include "../../string.h"
#include "../../test/test.h"
#include "../instr.h"
#include "../mbuilder.h"
#include "../module.h"
#include "vm.test.h"

namespace {

  using namespace Ant;
  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::ModuleBuilder";

#define REG_ALLOC_NORMAL_SEQ(reg) \
  if(passed) { \
    b.addProcInstr(p, CPI8Instr(0, reg)); \
    b.addProcInstr(p, MULInstr(reg, reg, reg)); \
    b.addProcInstr(p, DECInstr(reg)); \
    b.addProcInstr(p, JGInstr(reg, reg, 0)); \
    b.addProcInstr(p, LDEInstr(reg, reg, reg)); \
    b.addProcInstr(p, LDBInstr(reg, 0, reg)); \
    b.addProcInstr(p, STEInstr(reg, reg, reg)); \
    b.addProcInstr(p, STBInstr(reg, reg, 0)); \
  }

#define REG_ALLOC_ERROR_SEQ(reg, exc) \
  ASSERT_THROW({b.addProcInstr(p, CPI8Instr(0, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, CPBInstr(reg, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, MULInstr(reg, reg, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, DECInstr(reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, JGInstr(reg, reg, 0));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, LDEInstr(reg, reg, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, LDBInstr(reg, 0, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, LDRInstr(reg, 0, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, STEInstr(reg, reg, reg));}, exc); \
  ASSERT_THROW({b.addProcInstr(p, STBInstr(reg, reg, 0));}, exc);

  bool testRegAllocation() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId vt = b.addVarType(8);
      RegId r1 = b.addReg(0, vt);
      RegId r2 = b.addReg(0, vt);
      RegId r3 = b.addReg(0, vt, 0);
      ProcTypeId pt = b.addProcType(0, r1);
      ProcId p = b.addProc(0, pt);

      REG_ALLOC_NORMAL_SEQ(r1);
      REG_ALLOC_ERROR_SEQ(r2, OperationException);
      b.addProcInstr(p, PUSHInstr(r2));
      REG_ALLOC_NORMAL_SEQ(r2);
      b.addProcInstr(p, PUSHInstr(r2));
      REG_ALLOC_NORMAL_SEQ(r2);
      b.addProcInstr(p, POPInstr());
      REG_ALLOC_NORMAL_SEQ(r2);
      b.addProcInstr(p, POPInstr());
      REG_ALLOC_ERROR_SEQ(r2, OperationException);

      ASSERT_THROW({b.addProcInstr(p, PUSHInstr(r3));}, TypeException);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "regAllocation", passed);
  }

#define REG_EXIST_NORMAL_SEQ(reg) \
  if(passed) { \
    b.addProcInstr(p, PUSHInstr(reg)); \
    b.addProcInstr(p, PUSHRInstr(reg)); \
    REG_ALLOC_NORMAL_SEQ(reg); \
  }

#define REG_EXIST_ERROR_SEQ(reg, exc) \
  REG_ALLOC_ERROR_SEQ(reg, exc);

  bool testRegExistence() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId vt = b.addVarType(8);
      RegId r = b.addReg(0, vt), n = r + 1;
      ProcTypeId pt = b.addProcType(0, r);
      ProcId p = b.addProc(0, pt);

      REG_EXIST_NORMAL_SEQ(r);
      REG_EXIST_ERROR_SEQ(n, NotFoundException);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "regExistence", passed);
  }

  bool testStackConsistency() {
    bool passed = true;
    Module m;

    try {
      ModuleBuilder b;
      VarTypeId vt = b.addVarType(8);
      RegId r = b.addReg(0, vt);
      ProcTypeId pt = b.addProcType(0, r);
      ProcId p = b.addProc(0, pt);

      ASSERT_THROW({b.addProcInstr(p, JNZInstr(r, -1));}, RangeException);
      b.addProcInstr(p, JNZInstr(r, 2));
      ASSERT_THROW({b.addProcInstr(p, PUSHInstr(r));}, OperationException);
      b.addProcInstr(p, DECInstr(r));
      ASSERT_THROW({b.addProcInstr(p, POPInstr());}, OperationException);
      ASSERT_THROW(b.createModule(m), OperationException);
      b.addProcInstr(p, PUSHInstr(r));
      b.addProcInstr(p, JNZInstr(r, 3));
      b.addProcInstr(p, PUSHInstr(r));
      ASSERT_THROW({b.addProcInstr(p, DECInstr(r));}, OperationException);
      b.addProcInstr(p, POPInstr());
      b.addProcInstr(p, JNZInstr(r, 4));
      b.addProcInstr(p, PUSHInstr(r));
      ASSERT_THROW({b.addProcInstr(p, JNZInstr(r, -1));}, RangeException);
      ASSERT_THROW({b.addProcInstr(p, JNZInstr(r, 2));}, RangeException);
      b.addProcInstr(p, JNZInstr(r, 1));
      ASSERT_THROW(b.createModule(m), OperationException);
      b.addProcInstr(p, POPInstr());
      ASSERT_THROW(b.createModule(m), OperationException);
      b.addProcInstr(p, POPInstr());
      b.addProcInstr(p, PUSHInstr(r));
      b.addProcInstr(p, JNZInstr(r, 2));
      ASSERT_THROW({b.addProcInstr(p, POPInstr());}, OperationException);
      b.addProcInstr(p, JNZInstr(r, 0));
      b.addProcInstr(p, POPInstr());
      ASSERT_THROW({b.addProcInstr(p, POPInstr());}, OperationException);
      b.addProcInstr(p, PUSHInstr(r));
      b.addProcInstr(p, PUSHInstr(r));
      ASSERT_THROW({b.addProcInstr(p, POPLInstr(3));}, OperationException);
      b.addProcInstr(p, POPLInstr(2));
      b.addProcInstr(p, POPLInstr(0));
      ASSERT_THROW({b.addProcInstr(p, PUSHHInstr(-1));}, RangeException);
      b.addProcInstr(p, PUSHHInstr(2));
      ASSERT_THROW({b.addProcInstr(p, POPInstr());}, OperationException);
      b.addProcInstr(p, THROWInstr());
      b.addProcInstr(p, POPInstr());
      b.createModule(m);
    }
    catch(...) { passed = false; }

    IGNORE_THROW(m.drop());

    return printTestResult(subj, "stackConsistency", passed);
  }

  bool testEltConsistency() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId vt1 = b.addVarType(8);
      VarTypeId vt2 = b.addVarType(1);
      VarTypeId vt3 = b.addVarType(1);
      RegId r1 = b.addReg(0, vt1, 2);
      RegId r2 = b.addReg(0, vt2, 3);
      RegId r3 = b.addReg(0, vt3, 4);
      ProcTypeId pt = b.addProcType(0, r1);
      ProcId p = b.addProc(0, pt);

      b.addProcInstr(p, PUSHInstr(r2));
      b.addProcInstr(p, PUSHInstr(r3));
      ASSERT_THROW({b.addProcInstr(p, LDEInstr(r2, r2, r2));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, LDEInstr(r2, r1, r3));}, TypeException);
      b.addProcInstr(p, LDEInstr(r2, r1, r2));
      ASSERT_THROW({b.addProcInstr(p, STEInstr(r2, r2, r2));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, STEInstr(r3, r2, r1));}, TypeException);
      b.addProcInstr(p, STEInstr(r2, r2, r1));
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "eltConsistency", passed);
  }

  bool testByteConsistency() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId vt = b.addVarType(7);
      RegId r = b.addReg(0, vt);
      ProcTypeId pt = b.addProcType(0, r);
      ProcId p = b.addProc(0, pt);

      ASSERT_THROW({b.addProcInstr(p, CPI8Instr(0, r));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, MULInstr(r, r, r));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, DECInstr(r));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, JGInstr(r, r, 0));}, TypeException);
      b.addProcInstr(p, LDBInstr(r, 6, r));
      ASSERT_THROW({b.addProcInstr(p, LDBInstr(r, 7, r));}, TypeException);
      b.addProcInstr(p, STBInstr(r, r, 6));
      ASSERT_THROW({b.addProcInstr(p, STBInstr(r, r, 7));}, TypeException);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "byteConsistency", passed);
  }

  bool testRefConsistency() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId vt1 = b.addVarType(7);
      VarTypeId vt2 = b.addVarType(0);
      b.addVarTypeVRef(vt2, vt1);
      b.addVarTypeVRef(vt2, vt1, 2);
      b.addVarTypeVRef(vt2, vt1, 0);
      RegId r1 = b.addReg(0, vt1);
      RegId r2 = b.addReg(0, vt1, 2);
      RegId r3 = b.addReg(0, vt1, 0);
      RegId r4 = b.addReg(0, vt2);
      ProcTypeId pt = b.addProcType(0, r1);
      ProcId p = b.addProc(0, pt);

      b.addProcInstr(p, PUSHRInstr(r2));
      b.addProcInstr(p, PUSHRInstr(r3));
      b.addProcInstr(p, PUSHInstr(r4));
      ASSERT_THROW({b.addProcInstr(p, LDRInstr(r4,0,r1));},OperationException);
      b.addProcInstr(p, PUSHRInstr(r1));
      b.addProcInstr(p, LDRInstr(r4, 0, r1));
      ASSERT_THROW({b.addProcInstr(p, LDRInstr(r4, 0, r2));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, LDRInstr(r4, 0, r3));}, TypeException);
      b.addProcInstr(p, LDRInstr(r4, 1, r1));
      b.addProcInstr(p, LDRInstr(r4, 1, r2));
      ASSERT_THROW({b.addProcInstr(p, LDRInstr(r4, 1, r3));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, LDRInstr(r4, 2, r1));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, LDRInstr(r4, 2, r2));}, TypeException);
      b.addProcInstr(p, LDRInstr(r4, 2, r3));
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "refConsistency", passed);
  }

  bool testCallConsistency() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId vt = b.addVarType(1);
      RegId r1 = b.addReg(0, vt);
      RegId r2 = b.addReg(0, vt);
      ProcTypeId pt1 = b.addProcType(0, r1);
      ProcTypeId pt2 = b.addProcType(0, r2);
      ProcId p1 = b.addProc(0, pt1);
      ProcId p2 = b.addProc(0, pt2);

      b.addProcInstr(p1, CALLInstr(p1));
      ASSERT_THROW({b.addProcInstr(p1, CALLInstr(p2));}, OperationException);
      b.addProcInstr(p1, PUSHInstr(r2));
      b.addProcInstr(p1, CALLInstr(p1));
      b.addProcInstr(p1, CALLInstr(p2));
      b.addProcInstr(p1, PUSHRInstr(r1));
      b.addProcInstr(p1, PUSHRInstr(r2));
      ASSERT_THROW({b.addProcInstr(p1, CALLInstr(p1));}, OperationException);
      ASSERT_THROW({b.addProcInstr(p1, CALLInstr(p2));}, OperationException);
      b.addProcInstr(p1, POPLInstr(1));
      b.addProcInstr(p1, CALLInstr(p1));
      b.addProcInstr(p1, CALLInstr(p2));
      b.addProcInstr(p1, POPInstr());
      b.addProcInstr(p1, CALLInstr(p1));
      ASSERT_THROW({b.addProcInstr(p1, CALLInstr(p2));}, OperationException);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "callConsistency", passed);
  }

  bool testFactorialVTypes(const Module &module) {
    VarType vtype;
    bool passed;

    passed = module.varTypeCount() == ModuleBuilder::RESERVED_VAR_TYPE_COUNT+1;

    if(passed) {
      module.varTypeById(ModuleBuilder::RESERVED_VAR_TYPE_COUNT, vtype);
      passed = vtype.bytes == 8 && !vtype.vrefs.size() && !vtype.prefs.size();
    }

    ASSERT_THROW({module.varTypeById(ModuleBuilder::RESERVED_VAR_TYPE_COUNT+1,
				     vtype);}, NotFoundException);

    return passed;
  }

  bool testFactorialPTypes(const Module &module) {
    ProcType ptype;
    bool passed;

    passed = module.procTypeCount() == 1;

    if(passed) {
      module.procTypeById(0, ptype);
      passed = !ptype.flags && ptype.io == ModuleBuilder::RESERVED_REG_COUNT;
    }

    ASSERT_THROW({module.procTypeById(1, ptype);}, NotFoundException);

    return passed;
  }

  bool testFactorialRegs(const Module &module) {
    bool passed;
    VarSpec reg;

    passed = module.regCount() == ModuleBuilder::RESERVED_REG_COUNT + 2;
    module.regById(ModuleBuilder::RESERVED_REG_COUNT, reg);
    passed = passed && !reg.flags &&
      reg.vtype == ModuleBuilder::RESERVED_VAR_TYPE_COUNT && reg.count == 1;
    module.regById(ModuleBuilder::RESERVED_REG_COUNT + 1, reg);
    passed = passed && !reg.flags &&
      reg.vtype == ModuleBuilder::RESERVED_VAR_TYPE_COUNT && reg.count == 1;

    ASSERT_THROW({module.regById(ModuleBuilder::RESERVED_REG_COUNT + 2, reg);},
		 NotFoundException);

    return passed;
  }

#define NEXT_OPCODE(prefix) \
  if(passed) { \
    instr.set(&proc.code[i]); \
    i += instr.size(); \
    passed = passed && instr.opcode() == OPCODE_##prefix; \
}

#define NEXT_INSTR_D(prefix) \
  NEXT_OPCODE(prefix); \
  prefix##Instr &i##prefix = static_cast<prefix##Instr&>(instr);

#define NEXT_INSTR(prefix) \
  NEXT_OPCODE(prefix); \
  i##prefix = static_cast<prefix##Instr&>(instr);

  bool testFactorialInstrs(const Proc &proc) {
    bool passed = true;
    RegId io = ModuleBuilder::RESERVED_REG_COUNT, pr = io + 1;
    Instr instr;
    int i = 0;

    NEXT_INSTR_D(JNZ);
    passed = passed && iJNZ.it() == io;
    passed = passed && iJNZ.offset() == 3;
    NEXT_INSTR_D(CPI8);
    passed = passed && iCPI8.val() == 1; 
    passed = passed && iCPI8.to() == io;
    NEXT_OPCODE(RET);
    NEXT_INSTR_D(PUSH);
    passed = passed && iPUSH.reg() == pr;
    NEXT_INSTR(CPI8);
    passed = passed && iCPI8.val() == 1; 
    passed = passed && iCPI8.to() == pr;
    NEXT_INSTR_D(MUL);
    passed = passed && iMUL.operand1() == io;
    passed = passed && iMUL.operand2() == pr;
    passed = passed && iMUL.result() == pr;
    NEXT_INSTR_D(DEC);
    passed = passed && iDEC.it() == io;
    NEXT_INSTR(JNZ);
    passed = passed && iJNZ.it() == io;
    passed = passed && iJNZ.offset() == -2;
    NEXT_INSTR_D(CPB);
    passed = passed && iCPB.from() == pr;
    passed = passed && iCPB.to() == io;
    NEXT_OPCODE(POP);
    NEXT_OPCODE(RET);

    passed = passed && i == proc.code.size();

    return passed;
  }

  bool testFactorialProcs(const Module &module) {
    bool passed;
    Proc proc;

    passed = module.procCount() == 1; 

    if(passed) {
      module.procById(0, proc);
      passed = proc.flags == PFLAG_EXTERNAL;
      passed = passed && proc.ptype == 0;
      passed = passed && testFactorialInstrs(proc);
    }

    ASSERT_THROW({module.procById(1, proc);}, NotFoundException);

    return passed;
  }

  bool testFactorial() {
    bool passed;
    Module module;

    try {
      createFactorialModule(module);

      passed = testFactorialVTypes(module);
      passed = passed && testFactorialPTypes(module);
      passed = passed && testFactorialRegs(module);
      passed = passed && testFactorialProcs(module);
    }
    catch(...) { passed = false; }

    IGNORE_THROW(module.drop());

    return printTestResult(subj, "factorial", passed);
  }

  bool testQSortVTypes(const Module &module) {
    VarType vtype;
    bool passed;

    passed = module.varTypeCount() == ModuleBuilder::RESERVED_VAR_TYPE_COUNT+2;

    if(passed) {
      module.varTypeById(ModuleBuilder::RESERVED_VAR_TYPE_COUNT + 1, vtype);
      passed = vtype.vrefs.size() == 1;
      passed = passed && !vtype.vrefs[0].count &&
        vtype.vrefs[0].vtype == ModuleBuilder::RESERVED_VAR_TYPE_COUNT;
    }

    return passed;
  }

  bool testQSortPartInstrs(const Proc &proc) {
    bool passed = true;
    RegId io = ModuleBuilder::RESERVED_REG_COUNT;
    RegId l = io + 1, h = l + 1, a = h + 1, al = a + 1, ah = al + 1;
    Instr instr;
    int i = 0;

    NEXT_OPCODE(PUSH);
    NEXT_INSTR_D(CPB);
    passed = passed && iCPB.from() == io;
    passed = passed && iCPB.to() == l;
    NEXT_OPCODE(PUSH);
    NEXT_INSTR_D(LDB);
    passed = passed && iLDB.from() == io;
    passed = passed && iLDB.offset() == 8;
    passed = passed && iLDB.to() == h;
    NEXT_OPCODE(PUSHR);
    NEXT_INSTR_D(LDR);
    passed = passed && iLDR.from() == io;
    passed = passed && iLDR.vref() == 0;
    passed = passed && iLDR.to() == a;
    NEXT_OPCODE(PUSH);
    NEXT_OPCODE(PUSH);
    NEXT_INSTR_D(LDE);
    passed = passed && iLDE.from() == a;
    passed = passed && iLDE.elt() == h;
    passed = passed && iLDE.to() == ah;
    NEXT_INSTR(LDE);
    passed = passed && iLDE.from() == a;
    passed = passed && iLDE.elt() == l;
    passed = passed && iLDE.to() == al;
    NEXT_INSTR_D(JG);
    passed = passed && iJG.operand1() == al;
    passed = passed && iJG.operand2() == ah;
    passed = passed && iJG.offset() == 7;
    NEXT_OPCODE(PUSH);
    NEXT_INSTR(LDE);
    passed = passed && iLDE.from() == a;
    passed = passed && iLDE.elt() == io;
    passed = passed && iLDE.to() == h;
    NEXT_INSTR_D(STE);
    passed = passed && iSTE.from() == al;
    passed = passed && iSTE.to() == a;
    passed = passed && iSTE.elt() == io;
    NEXT_INSTR(STE);
    passed = passed && iSTE.from() == h;
    passed = passed && iSTE.to() == a;
    passed = passed && iSTE.elt() == l;
    NEXT_OPCODE(INC);
    NEXT_OPCODE(POP);
    NEXT_OPCODE(INC);
    NEXT_INSTR_D(JNG);
    passed = passed && iJNG.operand1() == l;
    passed = passed && iJNG.operand2() == h;
    passed = passed && iJNG.offset() == -9;
    NEXT_OPCODE(DEC);
    NEXT_INSTR_D(POPL);
    passed = passed && iPOPL.level() == 0;

    return passed;
  }

  bool testQSortQSortInstrs(const Proc &proc) {
    bool passed = true;
    RegId io = ModuleBuilder::RESERVED_REG_COUNT;
    RegId l = io + 1, h = l + 1, a = h + 1, al = a + 1, ah = al + 1;
    ProcId part = 0, qsort = 1;
    Instr instr;
    int i = 0;

    NEXT_OPCODE(PUSH);
    NEXT_INSTR_D(CPB);
    passed = passed && iCPB.from() == io;
    passed = passed && iCPB.to() == l;
    NEXT_OPCODE(PUSH);
    NEXT_INSTR_D(LDB);
    passed = passed && iLDB.from() == io;
    passed = passed && iLDB.offset() == 8;
    passed = passed && iLDB.to() == h;
    NEXT_INSTR_D(JG);
    passed = passed && iJG.operand1() == h;
    passed = passed && iJG.operand2() == l;
    passed = passed && iJG.offset() == 2;
    NEXT_OPCODE(RET);
    NEXT_INSTR_D(CALL);
    passed = passed && iCALL.proc() == part;
    NEXT_INSTR(CPB);
    passed = passed && iCPB.from() == io;
    passed = passed && iCPB.to() == h;
    NEXT_OPCODE(INC);
    NEXT_INSTR(CALL);
    passed = passed && iCALL.proc() == qsort;
    NEXT_INSTR(CPB);
    passed = passed && iCPB.from() == l;
    passed = passed && iCPB.to() == io;
    NEXT_OPCODE(DEC);
    NEXT_INSTR_D(STB);
    passed = passed && iSTB.from() == h;
    passed = passed && iSTB.to() == io;
    passed = passed && iSTB.offset() == 8;
    NEXT_INSTR(CALL);
    passed = passed && iCALL.proc() == qsort;
    NEXT_INSTR_D(POPL);
    passed = passed && iPOPL.level() == 0;

    return passed;
  }

  bool testQSortProcs(const Module &module) {
    bool passed;
    Proc proc;

    passed = module.procCount() == 2; 

    if(passed) {
      module.procById(0, proc);
      passed = passed && testQSortPartInstrs(proc);
      module.procById(1, proc);
      passed = passed && testQSortQSortInstrs(proc);
    }

    return passed;
  }

  bool testQSort() {
    bool passed = true;
    Module module;

    try {
      createQSortModule(module);

      passed = testQSortVTypes(module);
      passed = passed && testQSortProcs(module);
    }
    catch(...) { passed = false; }

    IGNORE_THROW(module.drop());

    return printTestResult(subj, "qsort", passed);
  }

  bool testEHProc1Instrs(const Proc &proc) {
    bool passed = true;
    Instr instr;
    int i = 0;

    NEXT_OPCODE(JE);
    NEXT_OPCODE(PUSH);
    NEXT_OPCODE(THROW);
  }

  bool testEHProc2Instrs(const Proc &proc) {
    bool passed = true;
    RegId ed = 0, io = ModuleBuilder::RESERVED_REG_COUNT;
    ProcId func1 = 0;
    Instr instr;
    int i = 0;

    NEXT_OPCODE(CPI8);
    NEXT_INSTR_D(PUSHH);
    passed = passed && iPUSHH.offset() == 11;
    NEXT_INSTR(PUSHH);
    passed = passed && iPUSHH.offset() == 4;
    NEXT_OPCODE(CALL);
    NEXT_OPCODE(CPI8);
    NEXT_INSTR_D(JMP);
    passed = passed && iJMP.offset() == 5;
    NEXT_OPCODE(CPB);
    NEXT_OPCODE(MUL);
    NEXT_INSTR_D(CALL);
    NEXT_OPCODE(CPI8);
    NEXT_OPCODE(POP);
    NEXT_INSTR(JMP);
    passed = passed && iJMP.offset() == 3;
    NEXT_OPCODE(CPI8);
    NEXT_OPCODE(THROW);
  }

  bool testEH() {
    bool passed;
    Module module;

    try {
      createEHTestModule(module);

      passed = module.procCount() == 2; 

      if(passed) {
        Proc proc;
        module.procById(0, proc);
        passed = passed && testEHProc1Instrs(proc);
        module.procById(1, proc);
        passed = passed && testEHProc2Instrs(proc);
      }
    }
    catch(...) { passed = false; }

    IGNORE_THROW(module.drop());

    return printTestResult(subj, "EH", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testModuleBuilder() {
        bool passed;

        passed = testRegExistence();
        passed = passed && testRegAllocation();
        passed = passed && testStackConsistency();
        passed = passed && testEltConsistency();
        passed = passed && testByteConsistency();
        passed = passed && testRefConsistency();
        passed = passed && testCallConsistency();
        passed = passed && testFactorial();
        passed = passed && testQSort();
        passed = passed && testEH();

        return passed;
      }

    }
  }
}
