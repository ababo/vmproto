#ifndef __VM_VMDEFS_INCLUDED__
#define __VM_VMDEFS_INCLUDED__

#include <cstddef>
#include <stdint.h>
#include <vector>

namespace Ant {
  namespace VM {

    typedef uint32_t VarTypeId;
    typedef uint32_t ProcTypeId;
    typedef uint32_t ProcId;
    typedef uint32_t RegId;

    typedef uint8_t VMCodeByte;
    typedef const VMCodeByte *VMCode;

    struct Variable {};

    template<bool Fixed> struct SVPartFixed { size_t refCount; };
    template<bool InStack> struct SVPartStack { size_t elmCount; };
    template<uint32_t Bytes> struct SVPartBytes { uint8_t bytes[Bytes]; };
    template<uint32_t VRefs> struct SVPartVRefs { Variable *vrefs[VRefs]; };
    template<uint32_t PRefs> struct SVPartPRefs { void *prefs[PRefs]; };
    template<> struct SVPartFixed<true> {};
    template<> struct SVPartStack<true> {};
    template<> struct SVPartBytes<0> {};
    template<> struct SVPartVRefs<0> {};
    template<> struct SVPartPRefs<0> {};

    template<uint32_t Bytes, uint32_t VRefs, uint32_t PRefs>
      struct SVPartElt : SVPartBytes<Bytes>, SVPartVRefs<VRefs>,
                         SVPartPRefs<PRefs> {};

    template<uint32_t Bytes, uint32_t VRefs, uint32_t PRefs,
             size_t Count = 1, bool Fixed = true, bool InStack = true>
      struct SpecifiedVariable : Variable, SVPartFixed<Fixed>,
                                 SVPartStack<InStack> {
       SVPartElt<Bytes, VRefs, PRefs> elts[Count];
    };

    struct VarSpec {
      VarTypeId vtype;
      size_t count;
    };

    struct VarType {
      size_t bytes;
      std::vector<VarSpec> vrefs;
      std::vector<ProcTypeId> prefs;
    };

    enum ProcTypeFlag {
      PTFLAG_READER = 0x1,
      PTFLAG_WRITER = 0x2,
      PTFLAG_FIRST_RESERVED = 0x4
    };

    struct ProcType {
      uint32_t flags;
      RegId io;
    };

    enum ProcFlag {
      PFLAG_EXTERNAL = 0x1,
      PFLAG_FIRST_RESERVED = 0x2
    };

    struct Proc {
      uint32_t flags;
      ProcTypeId ptype;
      std::vector<VMCodeByte> code;
    };

    enum OpCode {
      OPCODE_ILL = 0,
      OPCODE_INC,
      OPCODE_DEC,
      OPCODE_ADD,
      OPCODE_SUB,
      OPCODE_MUL,
      OPCODE_JNZ,
      OPCODE_JUG,
      OPCODE_IMM1,
      OPCODE_IMM2,
      OPCODE_IMM4,
      OPCODE_IMM8,
      OPCODE_AST,
      OPCODE_ASTR,
      OPCODE_FST,
      OPCODE_CPB,
      OPCODE_CPBO,
      OPCODE_DREF,
      OPCODE_RET
    };

    template<uint8_t> class UOInstr;
    typedef UOInstr<OPCODE_INC> INCInstr;
    typedef UOInstr<OPCODE_DEC> DECInstr;

    template<uint8_t> class BOInstr;
    typedef BOInstr<OPCODE_ADD> ADDInstr;
    typedef BOInstr<OPCODE_SUB> SUBInstr;
    typedef BOInstr<OPCODE_MUL> MULInstr;

    template<uint8_t> class UJInstr;
    typedef UJInstr<OPCODE_JNZ> JNZInstr;

    template<uint8_t> class BJInstr;
    typedef BJInstr<OPCODE_JUG> JUGInstr;

    template<uint8_t, class> class IMMInstr;
    typedef IMMInstr<OPCODE_IMM1, uint8_t> IMM1Instr;
    typedef IMMInstr<OPCODE_IMM2, uint16_t> IMM2Instr;
    typedef IMMInstr<OPCODE_IMM4, uint32_t> IMM4Instr;
    typedef IMMInstr<OPCODE_IMM8, uint64_t> IMM8Instr;

    template<uint8_t, bool> class ASTTInstr;
    typedef ASTTInstr<OPCODE_AST, false> ASTInstr;
    typedef ASTTInstr<OPCODE_ASTR, true> ASTRInstr;

    class FSTInstr;
    class CPBInstr;
    class CPBOInstr;
    class DREFInstr;
    class RETInstr;

  }
}

#endif // __VM_VMDEFS_INCLUDED__
