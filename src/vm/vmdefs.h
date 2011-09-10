#ifndef __VM_VMDEFS_INCLUDED__
#define __VM_VMDEFS_INCLUDED__

#include <cstddef>
#include <stdint.h>
#include <vector>

namespace Ant {
  namespace VM {

    typedef uint32_t VarTypeId;
    typedef uint32_t ProcId;
    typedef uint32_t RegId;

    typedef uint8_t VMCodeByte;
    typedef const VMCodeByte *VMCode;

    struct Variable {};

    template<uint32_t Bytes, uint32_t VRefs, uint32_t PRefs, size_t Count = 1,
             bool InHeap = false>
      struct SpecifiedVariable : public Variable {
      size_t refCount;
      size_t elmCount;
      struct {
        unsigned char bytes[Bytes];
        Variable *vrefs[VRefs];
        void *prefs[PRefs];
      } elts[Count];
    };

    template<uint32_t Bytes, size_t Count>
      struct SpecifiedVariable<Bytes, 0, 0, Count, false> : public Variable {
      struct {
        unsigned char bytes[Bytes];
      } elts[Count];
    };

    struct VarType {
      size_t bytes;
      std::vector<VarTypeId> vrefs;
      std::vector<VarTypeId> prefs;
    };

    struct Reg {
      VarTypeId vtype;
      size_t count;
    };

    struct Proc {
      uint32_t flags;
      RegId io;
      std::vector<VMCodeByte> code;
    };

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FUNCTION = 0x2,
      PFLAG_FIRST_RESERVED = 0x4
    };

    enum OpCode {
      OPCODE_ILL = 0,
      OPCODE_INC,
      OPCODE_DEC,
      OPCODE_ADD,
      OPCODE_SUB,
      OPCODE_MUL,
      OPCODE_AST,
      OPCODE_FST,
      OPCODE_MOVM8,
      OPCODE_MOVN8,
      OPCODE_JNZ,
      OPCODE_RET
    };

    template<uint8_t> class UOInstr;
    typedef UOInstr<OPCODE_INC> INCInstr;
    typedef UOInstr<OPCODE_DEC> DECInstr;

    template<uint8_t> class BOInstr;
    typedef BOInstr<OPCODE_ADD> ADDInstr;
    typedef BOInstr<OPCODE_SUB> SUBInstr;
    typedef BOInstr<OPCODE_MUL> MULInstr;

    class ASTInstr;
    class FSTInstr;
    class MOVM8Instr;
    class MOVN8Instr;
    class JNZInstr;
    class RETInstr;

  }
}

#endif // __VM_VMDEFS_INCLUDED__
