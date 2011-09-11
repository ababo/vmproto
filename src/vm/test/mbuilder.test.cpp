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

  bool testRegConsistency() {
    bool passed = true;

    try {
      ModuleBuilder b;
      VarTypeId t = b.addVarType(7);
      RegId r = b.addReg(t), n = r + 1;
      ProcId p = b.addProc(0, r);

      ASSERT_THROW({b.addProc(0, n);}, NotFoundException);
      ASSERT_THROW({b.addProcInstr(p, ASTInstr(n));}, NotFoundException);
      ASSERT_THROW({b.addProcInstr(p, IMM8Instr(0, r));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, IMM8Instr(0, n));}, NotFoundException);
      ASSERT_THROW({b.addProcInstr(p, CPBInstr(n, n));}, NotFoundException);
      ASSERT_THROW({b.addProcInstr(p, MULInstr(r, r, r));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, MULInstr(n, n, n));},NotFoundException);
      ASSERT_THROW({b.addProcInstr(p, DECInstr(r));}, TypeException);
      ASSERT_THROW({b.addProcInstr(p, DECInstr(n));}, NotFoundException);

      t = b.addVarType(8);
      n = b.addReg(t);

      ASSERT_THROW({b.addProcInstr(p, DECInstr(n));}, OperationException);
      b.addProcInstr(p, ASTInstr(n));
      b.addProcInstr(p, DECInstr(n));
      b.addProcInstr(p, ASTInstr(r));
      b.addProcInstr(p, DECInstr(n));
      b.addProcInstr(p, FSTInstr());
      b.addProcInstr(p, DECInstr(n));
      b.addProcInstr(p, FSTInstr());
      ASSERT_THROW({b.addProcInstr(p, DECInstr(n));}, OperationException);
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "regConsistency", passed);
  }

  bool testStackConsistency() {
    bool passed = true;
    Module m;

    try {
      ModuleBuilder b;
      VarTypeId t = b.addVarType(8);
      RegId r = b.addReg(t);
      ProcId p = b.addProc(0, r);

      ASSERT_THROW({b.addProcInstr(p, JNZInstr(r, -1));}, RangeException);
      b.addProcInstr(p, JNZInstr(r, 2));
      ASSERT_THROW({b.addProcInstr(p, ASTInstr(r));}, OperationException);
      b.addProcInstr(p, DECInstr(r));
      ASSERT_THROW({b.addProcInstr(p, FSTInstr());}, OperationException);
      ASSERT_THROW(b.createModule(m), OperationException);
      b.addProcInstr(p, ASTInstr(r));
      b.addProcInstr(p, JNZInstr(r, 3));
      b.addProcInstr(p, ASTInstr(r));
      ASSERT_THROW({b.addProcInstr(p, DECInstr(r));}, OperationException);
      b.addProcInstr(p, FSTInstr());
      b.addProcInstr(p, JNZInstr(r, 4));
      b.addProcInstr(p, ASTInstr(r));
      ASSERT_THROW({b.addProcInstr(p, JNZInstr(r, -1));}, RangeException);
      ASSERT_THROW({b.addProcInstr(p, JNZInstr(r, 2));}, RangeException);
      b.addProcInstr(p, JNZInstr(r, 1));
      ASSERT_THROW(b.createModule(m), OperationException);
      b.addProcInstr(p, FSTInstr());
      ASSERT_THROW(b.createModule(m), OperationException);
      b.addProcInstr(p, FSTInstr());
      b.addProcInstr(p, ASTInstr(r));
      b.addProcInstr(p, JNZInstr(r, 2));
      ASSERT_THROW({b.addProcInstr(p, FSTInstr());}, OperationException);
      b.addProcInstr(p, JNZInstr(r, 0));
      b.addProcInstr(p, FSTInstr());
      b.createModule(m);
    }
    catch(...) { passed = false; }

    IGNORE_THROW(m.drop());

    return printTestResult(subj, "stackConsistency", passed);
  }

  bool testFactorialVTypes(const Module &module) {
    VarType vtype;
    bool passed;
    
    passed = module.varTypeCount() == 1;

    if(passed) {
      module.varTypeById(0, vtype);
      passed = vtype.bytes == 8 && !vtype.vrefs.size() && !vtype.prefs.size();
    }

    ASSERT_THROW({module.varTypeById(1, vtype);}, NotFoundException);

    return passed;
  }

  bool testFactorialRegs(const Module &module) {
    bool passed;
    Reg reg;

    passed = module.regCount() == 2;
    module.regById(0, reg);
    passed = passed && reg.vtype == 0 && reg.count == 1;
    module.regById(1, reg);
    passed = passed && reg.vtype == 0 && reg.count == 1;

    ASSERT_THROW({module.regById(2, reg);}, NotFoundException);

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
    RegId io = 0, pr = 1;
    Instr instr;
    int i = 0;

    NEXT_INSTR_D(JNZ);
    passed = passed && iJNZ.it() == io;
    passed = passed && iJNZ.offset() == 3;
    NEXT_INSTR_D(IMM8);
    passed = passed && iIMM8.val() == 1; 
    passed = passed && iIMM8.to() == io;
    NEXT_OPCODE(RET);
    NEXT_INSTR_D(AST);
    passed = passed && iAST.reg() == pr;
    NEXT_INSTR(IMM8);
    passed = passed && iIMM8.val() == 1; 
    passed = passed && iIMM8.to() == pr;
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
    NEXT_OPCODE(FST);
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
      passed = proc.flags == PFLAG_EXTERNAL | PFLAG_FUNCTION;
      passed = passed && proc.io == 0;
      passed = passed && testFactorialInstrs(proc);
    }

    ASSERT_THROW({module.procById(1, proc);}, NotFoundException);

    return passed;
  }

  bool testFactorial() {
    bool passed = true;

    try {
      Module module;
      createFactorialModule(module);

      passed = testFactorialVTypes(module);
      passed = passed && testFactorialRegs(module);
      passed = passed && testFactorialProcs(module);

      module.drop();
    }
    catch(...) { passed = false; }

    return printTestResult(subj, "factorial", passed);
  }

}

namespace Ant {
  namespace VM {
    namespace Test {

      bool testModuleBuilder() {
        bool passed;

        passed = testRegConsistency();
        passed = passed && testStackConsistency();
        passed = passed && testFactorial();

        return passed;
      }

    }
  }
}
