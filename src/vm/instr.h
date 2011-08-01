#ifndef __VM_INSTR_INCLUDED__
#define __VM_INSTR_INCLUDED__

namespace Ant {
  namespace VM {

#define MAX_INSTR_SIZE 10

    typedef uint16_t RegId;

    enum InstrType {
      INSTR_ILL = 0,
      INSTR_MOV8L
    };
 
    class Instr {
    public:
      virtual size_t size() = 0;

    protected:
      uint8_t type;
      uint8_t data[MAX_INSTR_SIZE - 1];
    };

    class MOV8LInstr {
    public:
      uint16_t from();
      uint16_t to();
      void setFrom(RegId from);
      void setTo(RegId to);
    };

  }
}

#endif // __VM_INSTR_INCLUDED__
