#ifndef __VM_INSTR_INCLUDED__
#define __VM_INSTR_INCLUDED__

#include <stddef.h>
#include <stdint.h>

#include "module.h"

namespace Ant {
  namespace VM {

    typedef unsigned int RegId;

    enum OpCode {
      OPCODE_ILL = 0
    };

#define MAX_INSTR_SIZE 13

    class Instr {
    public:
      Instr(BinaryCode *code);

      OpCode opcode() const {
        return static_cast<OpCode>(op); }

      size_t size() const;

    protected:
      Instr() {}

      uint8_t op;
      uint8_t data[MAX_INSTR_SIZE - 1];
    };

    class AFRMInstr : public Instr {
    public:
      AFRMInstr(FrameId frame);

      FrameId frame() const;
    };

    class FFRMInstr : public Instr {
    public:
      FFRMInstr();
    };

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
      JNZInstr(int offset);

      int offset() const;
    };

    class RETInstr : public Instr {
    public:
      RETInstr();
    };

  }
}

#endif // __VM_INSTR_INCLUDED__
