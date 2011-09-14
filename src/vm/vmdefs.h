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
    template<> struct SVPartFixed<true> {};

    template<bool InStack> struct SVPartStack { size_t elmCount; };
    template<> struct SVPartStack<true> {};

    template<uint32_t Bytes> struct SVPartBytes { uint8_t bytes[Bytes]; };
    template<> struct SVPartBytes<0> {};

    template<uint32_t VRefs> struct SVPartVRefs { Variable *vrefs[VRefs]; };
    template<> struct SVPartVRefs<0> {};

    template<uint32_t PRefs> struct SVPartPRefs { void *prefs[PRefs]; };
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
      OPCODE_CPI1,
      OPCODE_CPI2,
      OPCODE_CPI4,
      OPCODE_CPI8,
      OPCODE_ALS,
      OPCODE_ALSR,
      OPCODE_FRS,
      OPCODE_CPB,
      OPCODE_LDB,
      OPCODE_LDR,
      OPCODE_RET
    };

    template<uint8_t> class UOInstrT;
    typedef UOInstrT<OPCODE_INC> INCInstr;
    typedef UOInstrT<OPCODE_DEC> DECInstr;

    template<uint8_t> class BOInstrT;
    typedef BOInstrT<OPCODE_ADD> ADDInstr;
    typedef BOInstrT<OPCODE_SUB> SUBInstr;
    typedef BOInstrT<OPCODE_MUL> MULInstr;

    template<uint8_t> class UJInstrT;
    typedef UJInstrT<OPCODE_JNZ> JNZInstr;

    template<uint8_t> class BJInstrT;
    typedef BJInstrT<OPCODE_JUG> JUGInstr;

    template<uint8_t, class> class CPIInstrT;
    typedef CPIInstrT<OPCODE_CPI1, uint8_t> CPI1Instr;
    typedef CPIInstrT<OPCODE_CPI2, uint16_t> CPI2Instr;
    typedef CPIInstrT<OPCODE_CPI4, uint32_t> CPI4Instr;
    typedef CPIInstrT<OPCODE_CPI8, uint64_t> CPI8Instr;

    template<uint8_t, bool> class ALSInstrT;
    typedef ALSInstrT<OPCODE_ALS, false> ALSInstr;
    typedef ALSInstrT<OPCODE_ALSR, true> ALSRInstr;

    class FRSInstr;
    class CPBInstr;
    class LDBInstr;
    class LDRInstr;
    class RETInstr;

  }
}

#endif // __VM_VMDEFS_INCLUDED__
