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

      static RegId assertRegExists(const ModuleBuilder &mbuilder, RegId reg);
      static RegId assertRegAllocated(const ModuleBuilder &mbuilder,
                                      ProcId proc, RegId reg);
      static RegId assertRegHasBytes(const ModuleBuilder &mbuilder,
                                     ProcId proc, size_t minBytes, RegId reg);
      static void applyStackAlloc(ModuleBuilder &mbuilder, ProcId proc,
                                  RegId reg);
      static void applyStackFree(ModuleBuilder &mbuilder, ProcId proc);
      static void applyInstrOffset(ModuleBuilder &mbuilder, ProcId proc,
                                   int offset);
      static void applyDefault(ModuleBuilder &mbuilder, ProcId proc);

      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const;

      uint8_t op;
      uint8_t dat[MAX_INSTR_SIZE - 1];
    };

    template<uint8_t OP> class UOInstr : public Instr {
      friend class Instr;
    public:
      UOInstr(RegId operand) { op = OP; setParam(operand); }

      size_t size() const { return Instr::size(1); }
      RegId operand() const { return RegId(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, 8, operand());
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
        Instr::assertRegHasBytes(mbuilder, proc, 8, operand1());
        Instr::assertRegHasBytes(mbuilder, proc, 8, operand2());
        Instr::assertRegHasBytes(mbuilder, proc, 8, result());
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
        Instr::assertRegHasBytes(mbuilder, proc, sizeof(VAL), to());
        Instr::applyDefault(mbuilder, proc);
      }
    };

    typedef IMMInstr<OPCODE_IMM1, uint8_t> IMM1Instr;
    typedef IMMInstr<OPCODE_IMM2, uint16_t> IMM2Instr;
    typedef IMMInstr<OPCODE_IMM4, uint32_t> IMM4Instr;
    typedef IMMInstr<OPCODE_IMM8, uint64_t> IMM8Instr;

    class ASTInstr : public Instr {
      friend class Instr;
    public:
      ASTInstr(RegId reg) { op = OPCODE_AST; setParam(reg); }

      size_t size() const { return Instr::size(1); }
      RegId reg() const { return RegId(getParam(0)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        RegId r = reg();
        Instr::assertRegExists(mbuilder, r);
        Instr::applyStackAlloc(mbuilder, proc, r);
      }
    };

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

    class MOVN8Instr : public Instr {
      friend class Instr;
    public:
      MOVN8Instr(RegId from, RegId to) {
        op = OPCODE_MOVN8; set2Params(from, to);
      }

      size_t size() const { return Instr::size(2); }
      RegId from() const { return RegId(getParam(0)); }
      RegId to() const { return RegId(getParam(1)); }
      bool breaks() const { return false; }
      bool jumps() const { return false; }
      size_t jumpIndex(size_t) const { return 0; }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, 8, from());
        Instr::assertRegHasBytes(mbuilder, proc, 8, to());
        Instr::applyDefault(mbuilder, proc);
      }
    };

    class JNZInstr : public Instr {
      friend class Instr;
    public:
      JNZInstr(RegId it, int offset) {
        op = OPCODE_JNZ; set2Params2(it, offset); }

      size_t size() const { return Instr::size(2); }
      RegId it() const { return RegId(getParam(0)); }
      int offset() const { return int(getParam2(1)); }
      bool breaks() const { return false; }
      bool jumps() const { return true; }
      size_t jumpIndex(size_t index) const {
        return size_t(ptrdiff_t(index) + offset()); }

    protected:
      void assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
        Instr::assertRegHasBytes(mbuilder, proc, 8, it());
        Instr::applyInstrOffset(mbuilder, proc, offset());
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
