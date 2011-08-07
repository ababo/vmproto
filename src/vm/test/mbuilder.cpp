#include "../../common/string.h"
#include "../../test/all.h"
#include "../mbuilder.h"
#include "../module.h"

namespace Ant {
  namespace VM {
    namespace Test {

      void createFactorialModule(Module &module) {
        ModuleBuilder builder;

        VarTypeId wordType = builder.addVarType(1, 8);

        RegId io = builder.addReg(wordType);
        RegId pr = builder.addReg(wordType);

        ProcId proc = builder.addProc(PFLAG_EXTERNAL | PFLAG_FUNCTION, io);
        builder.addProcInstr(proc, ASTInstr(pr));
        builder.addProcInstr(proc, MOVM8Instr(1, pr));
        builder.addProcInstr(proc, MULInstr(io, pr, pr));
        builder.addProcInstr(proc, DECInstr(io));
        builder.addProcInstr(proc, JNZInstr(-2));
        builder.addProcInstr(proc, MOVN8Instr(pr, io));
        builder.addProcInstr(proc, FSTInstr());
        builder.addProcInstr(proc, RETInstr());

        builder.createModule(module);
      }

    }
  }
}

namespace {

  using namespace Ant::VM;
  using namespace Ant::Test;
  using namespace Ant::Common;
  using namespace Ant::VM::Test;

  const String subj = "Ant::VM::ModuleBuilder";

#define NEXT_OPCODE(prefix) \
  if(passed) { \
    instr.set(&proc.code[i]); \
    i += instr.size(); \
    passed = passed && instr.opcode() == OPCODE_##prefix; \
}

#define NEXT_INSTR(prefix) \
  NEXT_OPCODE(prefix); \
  prefix##Instr &i##prefix = static_cast<prefix##Instr&>(instr);

  bool testFactorial() {
    bool passed = true;
    Module module;

    try {
      createFactorialModule(module);

      passed = module.varTypeCount() == 1;
      if(passed) {
        VarType vtype;
        module.varTypeById(0, vtype);
        passed = vtype.count == 1 && vtype.bytes == 8;
        passed = passed && !vtype.vrefs.size() && !vtype.prefs.size();
      }

      passed = passed && module.regCount() == 2;
      passed = passed && module.regTypeById(RESERVED_REGS_COUNT) == 0;
      passed = passed && module.regTypeById(RESERVED_REGS_COUNT + 1) == 0;

      passed = passed && module.procCount() == 1; 
      if(passed) {
        Proc proc;
        module.procById(0, proc);
        passed = proc.flags == PFLAG_EXTERNAL | PFLAG_FUNCTION;
        passed = passed && proc.io == RESERVED_REGS_COUNT;

        int i = 0;
        Instr instr;
        NEXT_INSTR(AST);
        passed = passed && iAST.reg() == RESERVED_REGS_COUNT + 1;
        NEXT_INSTR(MOVM8);
        passed = passed && iMOVM8.val() == 1; 
        passed = passed && iMOVM8.to() == RESERVED_REGS_COUNT + 1;
        NEXT_INSTR(MUL);
        passed = passed && iMUL.factor1() == RESERVED_REGS_COUNT;
        passed = passed && iMUL.factor2() == RESERVED_REGS_COUNT + 1;
        passed = passed && iMUL.product() == RESERVED_REGS_COUNT + 1;
        NEXT_INSTR(DEC);
        passed = passed && iDEC.it() == RESERVED_REGS_COUNT;
        NEXT_INSTR(JNZ);
        passed = passed && iJNZ.offset() == -2;
        NEXT_INSTR(MOVN8);
        passed = passed && iMOVN8.from() == RESERVED_REGS_COUNT + 1;
        passed = passed && iMOVN8.to() == RESERVED_REGS_COUNT;
        NEXT_OPCODE(FST);
        NEXT_OPCODE(RET);
        passed = passed && i == proc.code.size();
      }

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

        passed = testFactorial();

        return passed;
      }

    }
  }
}
