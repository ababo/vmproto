#ifndef __VM_VMDEFS_INCLUDED__
#define __VM_VMDEFS_INCLUDED__

#include <cstddef>
#include <vector>

namespace Ant {
  namespace VM {

    typedef unsigned int VarTypeId;
    typedef unsigned int ProcId;
    typedef unsigned int RegId;

    typedef unsigned char VMCodeByte;
    typedef const VMCodeByte *VMCode;
    typedef const void *NativeCode;

    struct Variable {};

    template<size_t Count, size_t Bytes, size_t VRefs, size_t PRefs>
      struct FixedVariable : public Variable {
      struct {
        unsigned char bytes[Bytes];
        Variable *vrefs[VRefs];
        NativeCode prefs[PRefs];
      } elts[Count];
    };

    template<size_t Count, size_t Bytes>
      struct FixedVariable<Count, Bytes, 0, 0> : public Variable {
      struct {
        unsigned char bytes[Bytes];
      } elts[Count];
    };

    struct VarType {
      size_t count;
      size_t bytes;
      std::vector<VarTypeId> vrefs;
      std::vector<VarTypeId> prefs;
    };

    struct Proc {
      unsigned int flags;
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
