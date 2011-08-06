#ifndef __VM_INSTR_INCLUDED__
#define __VM_INSTR_INCLUDED__

#include <cstddef>
#include <stdint.h>

#include "module.h"

namespace Ant {
  namespace VM {

    enum OpCode {
      OPCODE_ILL = 0,
      OPCODE_AST,
      OPCODE_FST,
      OPCODE_MOVM8,
      OPCODE_MOVN8,
      OPCODE_MUL,
      OPCODE_DEC,
      OPCODE_JNZ,
      OPCODE_RET
    };

#define MAX_INSTR_SIZE 11

    class Instr {
    public:
      Instr() : op(OPCODE_ILL) {}
      Instr(VMCode code) { set(code); }

      void set(VMCode code);

      OpCode opcode() const { return OpCode(op); }

      size_t size() const;
      VMCode data() const { return dat; }

    protected:
      void setParam(uint64_t p);
      void setParam2(int64_t p);
      void set2Params(uint64_t p1, uint64_t p2);
      void set3Params(uint64_t p1, uint64_t p2, uint64_t p3);

      size_t size(int paramCount) const;
      uint64_t getParam(int index) const;
      int64_t getParam2(int index) const;

      static inline RegId assertRegId(RegId reg);
      static inline int assertInstrOffset(int offset);

      uint8_t op;
      uint8_t dat[MAX_INSTR_SIZE - 1];
    };

    class ASTInstr : public Instr {
    public:
      ASTInstr(RegId reg);

      size_t size() const { return Instr::size(1); }
      RegId reg() const { return RegId(getParam(0)); }
    };

    class FSTInstr : public Instr {
    public:
      FSTInstr();

      size_t size() const { return 1; }
    };

    class MOVM8Instr : public Instr {
    public:
      MOVM8Instr(uint64_t val, RegId to);

      size_t size() const { return Instr::size(2); }
      uint64_t val() const { return getParam(0); }
      RegId to() const { return RegId(getParam(1)); }
    };

    class MOVN8Instr : public Instr {
    public:
      MOVN8Instr(RegId from, RegId to);

      size_t size() const { return Instr::size(2); }
      RegId from() const { return RegId(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }
    };

    class MULInstr : public Instr {
    public:
      MULInstr(RegId factor1, RegId factor2, RegId product);

      size_t size() const { return Instr::size(3); }
      RegId factor1() const { return RegId(getParam(0)); }
      RegId factor2() const { return RegId(getParam(1)); }
      RegId product() const { return RegId(getParam(2)); }
    };

    class DECInstr : public Instr {
    public:
      DECInstr(RegId it);

      size_t size() const { return Instr::size(1); }
      RegId it() const { return RegId(getParam(0)); }
    };

    class JNZInstr : public Instr {
    public:
      JNZInstr(int offset);

      size_t size() const { return Instr::size(1); }
      int offset() const { return int(getParam2(0)); }
    };

    class RETInstr : public Instr {
    public:
      RETInstr();

      size_t size() const { return 1; }
    };

  }
}

#endif // __VM_INSTR_INCLUDED__
