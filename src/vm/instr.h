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

      size_t size(int paramCount) const;
      uint64_t getParam(int index) const;
      int64_t getParam2(int index) const;

      static void assertRegExists(const ModuleBuilder &mbuilder, RegId reg);
      static void assertRegAllocated(const ModuleBuilder &mbuilder,
                                     ProcId proc, RegId reg);
      static void assertRegHasBytes(const ModuleBuilder &mbuilder,
                                    ProcId proc, RegId reg, uint32_t bytes);
      static void assertValidDeref(const ModuleBuilder &mbuilder, ProcId proc,
                                   RegId from, uint32_t vref, RegId to);
      static void applyStackAlloc(ModuleBuilder &mbuilder, ProcId proc,
                                  RegId reg, bool asRef);
      static void applyStackFree(ModuleBuilder &mbuilder, ProcId proc);
      static void applyInstrOffset(ModuleBuilder &mbuilder, ProcId proc,
                                   ptrdiff_t offset);
      static void applyDefault(ModuleBuilder &mbuilder, ProcId proc);

      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const;

      uint8_t op;
      uint8_t dat[MAX_INSTR_SIZE - 1];
    };

    template<uint8_t OP> class UOInstr : public Instr {
      friend class Instr;
    public:
      UOInstr(RegId it) { op = OP; setParam(it); }

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

    typedef UOInstr<OPCODE_INC> INCInstr;
    typedef UOInstr<OPCODE_DEC> DECInstr;

    template<uint8_t OP> class BOInstr : public Instr {
      friend class Instr;
    public:
      BOInstr(RegId operand1, RegId operand2, RegId result) {
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

    typedef BOInstr<OPCODE_ADD> ADDInstr;
    typedef BOInstr<OPCODE_SUB> SUBInstr;
    typedef BOInstr<OPCODE_MUL> MULInstr;

    template<uint8_t OP, class VAL> class IMMInstr : public Instr {
      friend class Instr;
    public:
      IMMInstr(VAL val, RegId to) {
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

    typedef IMMInstr<OPCODE_IMM1, uint8_t> IMM1Instr;
    typedef IMMInstr<OPCODE_IMM2, uint16_t> IMM2Instr;
    typedef IMMInstr<OPCODE_IMM4, uint32_t> IMM4Instr;
    typedef IMMInstr<OPCODE_IMM8, uint64_t> IMM8Instr;

    template<uint8_t OP> class UJInstr : public Instr {
      friend class Instr;
    public:
      UJInstr(RegId it, ptrdiff_t offset) {
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

    typedef UJInstr<OPCODE_JNZ> JNZInstr;

    template<uint8_t OP, bool REF> class ASTTInstr : public Instr {
      friend class Instr;
    public:
      ASTTInstr(RegId reg) { op = OP; setParam(reg); }

      size_t size() const { return Instr::size(1); }
      RegId reg() const { return RegId(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        RegId r = reg();
        Instr::assertRegExists(mbuilder, r);
        Instr::applyStackAlloc(mbuilder, proc, r, REF);
      }
    };

    typedef ASTTInstr<OPCODE_AST, false> ASTInstr;
    typedef ASTTInstr<OPCODE_ASTR, true> ASTRInstr;

    class FSTInstr : public Instr {
      friend class Instr;
    public:
      FSTInstr() { op = OPCODE_FST; }

      size_t size() const { return 1; }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::applyStackFree(mbuilder, proc);
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
        Instr::assertRegExists(mbuilder, from());
        Instr::assertRegExists(mbuilder, to());
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class CPBOInstr : public Instr {
      friend class Instr;
    public:
      CPBOInstr(RegId from, uint32_t offset, RegId to) {
        op = OPCODE_CPBO; set3Params(from, offset, to);
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

    class DREFInstr : public Instr {
      friend class Instr;
    public:
      DREFInstr(RegId from, uint32_t vref, RegId to) {
        op = OPCODE_DREF; set3Params(from, vref, to);
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
        Instr::assertValidDeref(mbuilder, proc, from(), vref(), to());
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
