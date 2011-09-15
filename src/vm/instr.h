#ifndef __VM_INSTR_INCLUDED__
#define __VM_INSTR_INCLUDED__

#include <cstddef>
#include <stdint.h>

#include "vmdefs.h"

namespace Ant {
  namespace VM {

#define MAX_INSTR_SIZE 11

    class ModuleBuilder;

    class Instr {
      friend class ModuleBuilder;
    public:
      Instr() : op(OPCODE_ILL) {}
      Instr(VMCode code) { set(code); }

      void set(VMCode code);

      OpCode opcode() const { return OpCode(op); }
      VMCode data() const { return &op; }

      size_t size() const;
      bool breaks() const;
      bool jumps() const;
      size_t jumpIndex(size_t index) const;

    protected:
      void setParam(uint64_t p);
      void set2Params(uint64_t p1, uint64_t p2);
      void set2Params2(uint64_t p1, int64_t p2);
      void set3Params(uint64_t p1, uint64_t p2, uint64_t p3);
      void set3Params2(uint64_t p1, uint64_t p2, int64_t p3);

      size_t size(int paramCount) const;
      uint64_t getParam(int index) const;
      int64_t getParam2(int index) const;

      static void assertRegExists(const ModuleBuilder &mbuilder, RegId reg);
      static void assertRegAllocated(const ModuleBuilder &mbuilder,
				     ProcId proc, RegId reg);
      static void assertRegHasBytes(const ModuleBuilder &mbuilder, ProcId proc,
				    RegId reg, uint32_t bytes);
      static void assertSameVarType(VarTypeId vtype1, VarTypeId vtype2);
      static void assertCompatibleEltCounts(size_t from, size_t to);
      static void assertProcExists(ModuleBuilder &mbuilder, ProcId proc);
      static void regSpec(const ModuleBuilder &mbuilder, ProcId proc,
                          RegId reg, VarSpec &vspec);
      static void vrefSpec(const ModuleBuilder &mbuilder, ProcId proc,
                           RegId reg, uint32_t vref, VarSpec &vspec);
      static void applyStackAlloc(ModuleBuilder &mbuilder, ProcId proc,
                                  RegId reg, bool asRef);
      static void applyStackFree(ModuleBuilder &mbuilder, ProcId proc,
				 uint32_t regs);
      static void applyInstrOffset(ModuleBuilder &mbuilder, ProcId proc,
                                   ptrdiff_t offset);
      static void applyDefault(ModuleBuilder &mbuilder, ProcId proc);

      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const;

      uint8_t op;
      uint8_t dat[MAX_INSTR_SIZE - 1];
    };

    template<uint8_t OP> class UOInstrT : public Instr {
      friend class Instr;
    public:
      UOInstrT(RegId it) { op = OP; setParam(it); }

      size_t size() const { return Instr::size(1); }
      RegId it() const { return RegId(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, it(), 8);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    typedef UOInstrT<OPCODE_INC> INCInstr;
    typedef UOInstrT<OPCODE_DEC> DECInstr;

    template<uint8_t OP> class BOInstrT : public Instr {
      friend class Instr;
    public:
      BOInstrT(RegId operand1, RegId operand2, RegId result) {
        op = OP; set3Params(operand1, operand2, result);
      }

      size_t size() const { return Instr::size(3); }
      RegId operand1() const { return RegId(getParam(0)); }
      RegId operand2() const { return RegId(getParam(1)); }
      RegId result() const { return RegId(getParam(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, operand1(), 8);
        Instr::assertRegHasBytes(mbuilder, proc, operand2(), 8);
        Instr::assertRegHasBytes(mbuilder, proc, result(), 8);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    typedef BOInstrT<OPCODE_ADD> ADDInstr;
    typedef BOInstrT<OPCODE_SUB> SUBInstr;
    typedef BOInstrT<OPCODE_MUL> MULInstr;

    template<uint8_t OP> class UJInstrT : public Instr {
      friend class Instr;
    public:
      UJInstrT(RegId it, ptrdiff_t offset) {
        op = OP; set2Params2(it, offset); }

      size_t size() const { return Instr::size(2); }
      RegId it() const { return RegId(getParam(0)); }
      ptrdiff_t offset() const { return ptrdiff_t(getParam2(1)); }
      bool breaks() const { return false; }
      bool jumps() const { return true; }
      size_t jumpIndex(size_t index) const {
        return size_t(ptrdiff_t(index) + offset()); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, it(), 8);
        Instr::applyInstrOffset(mbuilder, proc, offset());
      }
    };

    typedef UJInstrT<OPCODE_JNZ> JNZInstr;

    template<uint8_t OP> class BJInstrT : public Instr {
      friend class Instr;
    public:
      BJInstrT(RegId operand1, RegId operand2, ptrdiff_t offset) {
        op = OP; set3Params2(operand1, operand2, offset); }

      size_t size() const { return Instr::size(3); }
      RegId operand1() const { return RegId(getParam(0)); }
      RegId operand2() const { return RegId(getParam(1)); }
      ptrdiff_t offset() const { return ptrdiff_t(getParam2(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return true; }
      size_t jumpIndex(size_t index) const {
        return size_t(ptrdiff_t(index) + offset()); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, operand1(), 8);
        Instr::assertRegHasBytes(mbuilder, proc, operand2(), 8);
        Instr::applyInstrOffset(mbuilder, proc, offset());
      }
    };

    typedef BJInstrT<OPCODE_JG> JGInstr;
    typedef BJInstrT<OPCODE_JNG> JNGInstr;

    template<uint8_t OP, class VAL> class CPIInstrT : public Instr {
      friend class Instr;
    public:
      CPIInstrT(VAL val, RegId to) {
        op = OP; set2Params(val, to);
      }

      size_t size() const { return Instr::size(2); }
      VAL val() const { return VAL(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, to(), sizeof(VAL));
        Instr::applyDefault(mbuilder, proc);
      }
    };

    typedef CPIInstrT<OPCODE_CPI1, uint8_t> CPI1Instr;
    typedef CPIInstrT<OPCODE_CPI2, uint16_t> CPI2Instr;
    typedef CPIInstrT<OPCODE_CPI4, uint32_t> CPI4Instr;
    typedef CPIInstrT<OPCODE_CPI8, uint64_t> CPI8Instr;

    template<uint8_t OP, bool REF> class ALSInstrT : public Instr {
      friend class Instr;
    public:
      ALSInstrT(RegId reg) { op = OP; setParam(reg); }

      size_t size() const { return Instr::size(1); }
      RegId reg() const { return RegId(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyStackAlloc(mbuilder, proc, reg(), REF);
      }
    };

    typedef ALSInstrT<OPCODE_ALS, false> ALSInstr;
    typedef ALSInstrT<OPCODE_ALSR, true> ALSRInstr;

    class FRSInstr : public Instr {
      friend class Instr;
    public:
      FRSInstr() { op = OPCODE_FRS; }

      size_t size() const { return 1; }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyStackFree(mbuilder, proc, 1);
      }
    };

    class FRSNInstr : public Instr {
      friend class Instr;
    public:
      FRSNInstr(uint32_t regs) { op = OPCODE_FRSN; }

      size_t size() const { return Instr::size(1); }
      uint32_t regs() const { return uint32_t(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyStackFree(mbuilder, proc, regs());
      }
    };

    class CPBInstr : public Instr {
      friend class Instr;
    public:
      CPBInstr(RegId from, RegId to) {
        op = OPCODE_CPB; set2Params(from, to);
      }

      size_t size() const { return Instr::size(2); }
      RegId from() const { return RegId(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, from(), 1);
        Instr::assertRegHasBytes(mbuilder, proc, to(), 1);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class LDEInstr : public Instr {
      friend class Instr;
    public:
      LDEInstr(RegId from, RegId elt, RegId to) {
        op = OPCODE_LDE; set3Params(from, elt, to);
      }

      size_t size() const { return Instr::size(3); }
      RegId from() const { return RegId(getParam(0)); }
      RegId elt() const { return RegId(getParam(1)); }
      RegId to() const { return RegId(getParam(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
	VarSpec fvspec, tvspec;
	Instr::regSpec(mbuilder, proc, from(), fvspec);
	Instr::regSpec(mbuilder, proc, to(), tvspec);
	Instr::assertSameVarType(fvspec.vtype, tvspec.vtype);
	Instr::assertRegHasBytes(mbuilder, proc, elt(), 8);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class LDBInstr : public Instr {
      friend class Instr;
    public:
      LDBInstr(RegId from, uint32_t offset, RegId to) {
        op = OPCODE_LDB; set3Params(from, offset, to);
      }

      size_t size() const { return Instr::size(3); }
      RegId from() const { return RegId(getParam(0)); }
      uint32_t offset() const { return uint32_t(getParam(1)); }
      RegId to() const { return RegId(getParam(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, from(), offset() + 1);
        Instr::assertRegHasBytes(mbuilder, proc, to(), 1);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class LDRInstr : public Instr {
      friend class Instr;
    public:
      LDRInstr(RegId from, uint32_t vref, RegId to) {
        op = OPCODE_LDR; set3Params(from, vref, to);
      }

      size_t size() const { return Instr::size(3); }
      RegId from() const { return RegId(getParam(0)); }
      uint32_t vref() const { return uint32_t(getParam(1)); }
      RegId to() const { return RegId(getParam(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
	VarSpec fvspec, tvspec;
	Instr::vrefSpec(mbuilder, proc, from(), vref(), fvspec);
	Instr::regSpec(mbuilder, proc, to(), tvspec);
	Instr::assertSameVarType(fvspec.vtype, tvspec.vtype);
	Instr::assertCompatibleEltCounts(fvspec.count, tvspec.count);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class STEInstr : public Instr {
      friend class Instr;
    public:
      STEInstr(RegId from, RegId to, RegId elt) {
        op = OPCODE_STE; set3Params(from, to, elt);
      }

      size_t size() const { return Instr::size(3); }
      RegId from() const { return RegId(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }
      RegId elt() const { return RegId(getParam(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
	VarSpec fvspec, tvspec;
	Instr::regSpec(mbuilder, proc, from(), fvspec);
	Instr::regSpec(mbuilder, proc, to(), tvspec);
	Instr::assertSameVarType(fvspec.vtype, tvspec.vtype);
	Instr::assertRegHasBytes(mbuilder, proc, elt(), 8);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class STBInstr : public Instr {
      friend class Instr;
    public:
      STBInstr(RegId from, RegId to, uint32_t offset) {
        op = OPCODE_STB; set3Params(from, to, offset);
      }

      size_t size() const { return Instr::size(3); }
      RegId from() const { return RegId(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }
      uint32_t offset() const { return uint32_t(getParam(2)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
	Instr::assertRegHasBytes(mbuilder, proc, from(), 1);
        Instr::assertRegHasBytes(mbuilder, proc, to(), offset() + 1);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class CALLInstr : public Instr {
      friend class Instr;
    public:
      CALLInstr(ProcId proc) { op = OPCODE_CALL; }

      size_t size() const { return Instr::size(1); }
      ProcId proc() const { return ProcId(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertProcExists(mbuilder, proc);
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class RETInstr : public Instr {
      friend class Instr;
    public:
      RETInstr() { op = OPCODE_RET; }

      size_t size() const { return 1; }
      bool breaks() const { return true; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyDefault(mbuilder, proc);
      }
    };

  }
}

#endif // __VM_INSTR_INCLUDED__
