#ifndef __VM_VMDEFS_INCLUDED__
#define __VM_VMDEFS_INCLUDED__

#include <cstddef>
#include <stdint.h>
#include <vector>

#include "utils.h"

namespace Ant {
  namespace VM {

    const uint32_t MODULE_VTYPES_MAX = MB_UINT_MAX(2);
    const uint32_t MODULE_PTYPES_MAX = MB_UINT_MAX(2);
    const uint32_t MODULE_REGS_MAX = MB_UINT_MAX(2);
    const uint32_t MODULE_PROCS_MAX = MB_UINT_MAX(2);

    const size_t PROC_INSTR_MAX = MB_UINT_MAX(4);

    const ptrdiff_t INSTR_OFFSET_MIN = MB_INT_MIN(1);
    const ptrdiff_t INSTR_OFFSET_MAX = MB_INT_MAX(1);

    const uint32_t ELT_BYTES_MAX = MB_UINT_MAX(2);
    const uint32_t ELT_VREFS_MAX = MB_UINT_MAX(2);
    const uint32_t ELT_PREFS_MAX = MB_UINT_MAX(2);

#ifdef SHORT_PTR
    const size_t VAR_COUNT_MAX = 0xFFFF;
#else
    const size_t VAR_COUNT_MAX = MB_UINT_MAX(4);
#endif

    typedef uint32_t VarTypeId;
    typedef uint32_t ProcTypeId;
    typedef uint32_t ProcId;
    typedef uint32_t RegId;

    typedef uint8_t VMCodeByte;
    typedef const VMCodeByte *VMCode;

    const uint32_t PRESET_VAR_TYPE_COUNT = 1;
    const uint32_t PRESET_REG_COUNT = 1;

    const VarTypeId PRESET_VAR_TYPE_WORD = 0;

    const RegId PRESET_REG_ED = 0;

    struct Variable {};

    template<uint32_t Bytes> struct SVPartBytes { uint8_t bytes[Bytes]; };
    template<> struct SVPartBytes<0> {};

    template<uint32_t VRefs> struct SVPartVRefs { Variable *vrefs[VRefs]; };
    template<> struct SVPartVRefs<0> {};

    template<uint32_t PRefs> struct SVPartPRefs { void *prefs[PRefs]; };
    template<> struct SVPartPRefs<0> {};

    template<uint32_t Bytes, uint32_t VRefs, uint32_t PRefs>
      struct SVPartElt : SVPartBytes<Bytes>, SVPartVRefs<VRefs>,
                         SVPartPRefs<PRefs> {};

    template<uint32_t Bytes, uint32_t VRefs, uint32_t PRefs, size_t Count = 1>
      struct SVariable : Variable {
       SVPartElt<Bytes, VRefs, PRefs> elts[Count];
    };

    template<bool RefCount> struct SVCPartRefCount { size_t refCount; };
    template<> struct SVCPartRefCount<false> {};

    template<bool EltCount> struct SVCPartEltCount { size_t eltCount; };
    template<> struct SVCPartEltCount<false> {};

    template<uint32_t Bytes, uint32_t VRefs, uint32_t PRefs, size_t Count = 1,
      bool RefCount = false, bool EltCount = false>
     struct SVContainer : SVCPartEltCount<EltCount>,SVCPartRefCount<RefCount> {
       SVariable<Bytes, VRefs, PRefs, Count> var;
    };

    enum VarFlag {
      VFLAG_NON_FIXED_REF = 0x1,
      VFLAG_PERSISTENT = 0x2,
      VFLAG_THREAD_LOCAL = 0x4,
      VFLAG_FIRST_RESERVED = 0x8
    };

    struct VarSpec {
      uint32_t flags;
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

    enum RegKind { RK_VOID, RK_REF, RK_NOREF, RK_NOVOID };

    enum OpCode {
      OPCODE_ILL = 0, // ILLegal
      OPCODE_INC, // INCrement
      OPCODE_DEC, // DECrement
      OPCODE_ADD, // ADD
      OPCODE_SUB, // SUBtract
      OPCODE_MUL, // MULtiply
      OPCODE_JNZ, // Jump if Not Zero
      OPCODE_JG, // Jump if Greater
      OPCODE_JNG, // Jump if Not Greater
      OPCODE_JE, // Jump if Equal
      OPCODE_CPI1, // CoPy Immediate (1-byte) 
      OPCODE_CPI2, // CoPy Immediate (2-bytes)
      OPCODE_CPI4, // CoPy Immediate (4-bytes)
      OPCODE_CPI8, // CoPy Immediate (8-bytes)
      OPCODE_PUSH, // PUSH variable
      OPCODE_PUSHR, // PUSH Reference
      OPCODE_PUSHH, // PUSH exception Handler
      OPCODE_POP, // POP entity
      OPCODE_JMP, // unconditional JuMP
      OPCODE_CPB, // CoPy Bytes
      OPCODE_LDE, // LoaD array Element
      OPCODE_LDB, // LoaD structure Bytes
      OPCODE_LDR, // LoaD structure Reference
      OPCODE_STE, // STore array Element
      OPCODE_STB, // STore structure Bytes
      OPCODE_STR, // STore structure Reference
      OPCODE_CALL, // CALL procedure
      OPCODE_THROW, // THROW exception
      OPCODE_RET // RETurn
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
    typedef BJInstrT<OPCODE_JG> JGInstr;
    typedef BJInstrT<OPCODE_JNG> JNGInstr;
    typedef BJInstrT<OPCODE_JE> JEInstr;

    template<uint8_t, class> class CPIInstrT;
    typedef CPIInstrT<OPCODE_CPI1, uint8_t> CPI1Instr;
    typedef CPIInstrT<OPCODE_CPI2, uint16_t> CPI2Instr;
    typedef CPIInstrT<OPCODE_CPI4, uint32_t> CPI4Instr;
    typedef CPIInstrT<OPCODE_CPI8, uint64_t> CPI8Instr;

    template<uint8_t, bool> class PUSHInstrT;
    typedef PUSHInstrT<OPCODE_PUSH, false> PUSHInstr;
    typedef PUSHInstrT<OPCODE_PUSHR, true> PUSHRInstr;

    class PUSHHInstr;
    class POPInstr;
    class JMPInstr;
    class CPBInstr;
    class LDEInstr;
    class LDBInstr;
    class LDRInstr;
    class STEInstr;
    class STBInstr;
    class STRInstr;
    class CALLInstr;
    class THROWInstr;
    class RETInstr;

    enum VMExceptionCode {
      VMECODE_NULL_DEREF = -1,
    };

  }
}

#endif // __VM_VMDEFS_INCLUDED__
