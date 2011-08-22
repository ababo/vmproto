#ifndef __VM_INSTR_INCLUDED__
#define __VM_INSTR_INCLUDED__

#include <cstddef>
#include <stdint.h>

#include "runtime.h"

namespace Ant {
  namespace VM {

    enum OpCode {
      OPCODE_ILL = 0,
      OPCODE_AST,
      OPCODE_FST,
      OPCODE_MOVM8,
      OPCODE_MOVN8,
      OPCODE_UMUL,
      OPCODE_DEC,
      OPCODE_JNZ,
      OPCODE_RET
    };

#define MAX_INSTR_SIZE 11

    class ModuleBuilder;

    class Instr {
      friend class ModuleBuilder;
    public:
      Instr() : op(OPCODE_ILL) {}
      Instr(VMCode code) { set(code); }

      void set(VMCode code);

      OpCode opcode() const { return OpCode(op); }

      size_t size() const;
      VMCode data() const { return &op; }

    protected:
      void setParam(uint64_t p);
      void set2Params(uint64_t p1, uint64_t p2);
      void set2Params2(uint64_t p1, int64_t p2);
      void set3Params(uint64_t p1, uint64_t p2, uint64_t p3);

      size_t size(int paramCount) const;
      uint64_t getParam(int index) const;
      int64_t getParam2(int index) const;

      RegId assertRegExists(const ModuleBuilder &mbuilder, RegId reg) const;
      RegId assertRegHasBytes(const ModuleBuilder &mbuilder, size_t minBytes,
                              RegId reg) const;
      void applyStackAlloc(ModuleBuilder &mbuilder, ProcId proc) const;
      void applyStackFree(ModuleBuilder &mbuilder, ProcId proc) const;
      void applyInstrOffset(ModuleBuilder &mbuilder, ProcId proc,
                            int offset) const;

      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const;

      uint8_t op;
      uint8_t dat[MAX_INSTR_SIZE - 1];
    };

    class ASTInstr : public Instr {
      friend class Instr;
    public:
      ASTInstr(RegId reg) { op = OPCODE_AST; setParam(reg); }

      size_t size() const { return Instr::size(1); }
      RegId reg() const { return RegId(getParam(0)); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegExists(mbuilder, reg());
        Instr::applyStackAlloc(mbuilder, proc);
      }
    };

    class FSTInstr : public Instr {
      friend class Instr;
    public:
      FSTInstr() { op = OPCODE_FST; }

      size_t size() const { return 1; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyStackFree(mbuilder, proc);
      }
    };

    class MOVM8Instr : public Instr {
      friend class Instr;
    public:
      MOVM8Instr(uint64_t val, RegId to) {
        op = OPCODE_MOVM8; set2Params(val, to);
      }

      size_t size() const { return Instr::size(2); }
      uint64_t val() const { return getParam(0); }
      RegId to() const { return RegId(getParam(1)); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId) const {
        Instr::assertRegHasBytes(mbuilder, 8, to());
      }
    };

    class MOVN8Instr : public Instr {
      friend class Instr;
    public:
      MOVN8Instr(RegId from, RegId to) {
        op = OPCODE_MOVN8; set2Params(from, to);
      }

      size_t size() const { return Instr::size(2); }
      RegId from() const { return RegId(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId) const {
        Instr::assertRegHasBytes(mbuilder, 8, from());
        Instr::assertRegHasBytes(mbuilder, 8, to());
      }
    };

    class UMULInstr : public Instr {
      friend class Instr;
    public:
      UMULInstr(RegId factor1, RegId factor2, RegId product) {
        op = OPCODE_UMUL; set3Params(factor1, factor2, product);
      }

      size_t size() const { return Instr::size(3); }
      RegId factor1() const { return RegId(getParam(0)); }
      RegId factor2() const { return RegId(getParam(1)); }
      RegId product() const { return RegId(getParam(2)); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId) const {
        Instr::assertRegHasBytes(mbuilder, 8, factor1());
        Instr::assertRegHasBytes(mbuilder, 8, factor2());
        Instr::assertRegHasBytes(mbuilder, 8, product());
      }
    };

    class DECInstr : public Instr {
      friend class Instr;
    public:
      DECInstr(RegId it) { op = OPCODE_DEC; setParam(it); }

      size_t size() const { return Instr::size(1); }
      RegId it() const { return RegId(getParam(0)); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId) const {
        Instr::assertRegHasBytes(mbuilder, 8, it());
      }
    };

    class JNZInstr : public Instr {
      friend class Instr;
    public:
      JNZInstr(RegId it, int offset) {
        op = OPCODE_JNZ; set2Params2(it, offset); }

      size_t size() const { return Instr::size(2); }
      RegId it() const { return RegId(getParam(0)); }
      int offset() const { return int(getParam2(1)); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyInstrOffset(mbuilder, proc, offset());
      }
    };

    class RETInstr : public Instr {
      friend class Instr;
    public:
      RETInstr() { op = OPCODE_RET; }

      size_t size() const { return 1; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {}
    };

  }
}

#endif // __VM_INSTR_INCLUDED__
