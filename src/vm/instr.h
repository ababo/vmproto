#ifndef __VM_INSTR_INCLUDED__
#define __VM_INSTR_INCLUDED__

#include <stddef.h>
#include <stdint.h>

namespace Ant {
  namespace VM {

    typedef uint16_t RegId;

    enum OpCode {
      OPCODE_ILL = 0,
      OPCODE_MOVM8
    };

#define MAX_INSTR_SIZE 11

    class Instr {
    public:
      OpCode opcode() const {
        return static_cast<OpCode>(op); }

      virtual size_t size() const { return 0; };

    protected:
      uint8_t op;
      uint8_t data[MAX_INSTR_SIZE - 1];
    };

    static const RegId FIRST_CALL_REG = 16;
    static const RegId FIRST_LOCAL_REG = 32768;

    class MOVM8Instr : public Instr {
    public:
      MOVM8Instr(uint64_t val, RegId to);

      uint64_t val() const;
      RegId to() const;
    };

    class MOVN8Instr : public Instr {
    public:
      MOVN8Instr(RegId from, RegId to);

      RegId from() const;
      RegId to() const;
    };

    class MULInstr : public Instr {
    public:
      MULInstr(RegId factor1, RegId factor2, RegId product);

      RegId factor1() const;
      RegId factor2() const;
      RegId product() const;
    };

    class DECInstr : public Instr {
    public:
      DECInstr(RegId it);

      RegId it() const;
    };

    class JNZInstr : public Instr {
    public:
      JNZInstr(int16_t offset);

      int16_t offset() const;
    };

    class RETInstr : public Instr {
    public:
      RETInstr();
    };

  }
}

#endif // __VM_INSTR_INCLUDED__
