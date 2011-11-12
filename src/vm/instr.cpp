#include <sstream>

#include "../exception.h"
#include "../util.h"
#include "instr.h"
#include "mbuilder.h"
#include "util.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace Ant;

    void Instr::set(VMCode code) {
      size_t size = reinterpret_cast<const Instr*>(code)->size();
      
      if(!size)
        throw EncodingException();

      copy(code, code + size, &op);
    }

#define VIRTUAL_CASE(op, left, mod, call) \
    case OPCODE_##op: { const char *opcode = STRZ(op); \
      left static_cast<mod op##Instr&>(*this).call; } break;

#define VIRTUAL_CALL(left, mod, call, def) \
    switch(op) { \
      VIRTUAL_CASE(INC, left, mod, call); \
      VIRTUAL_CASE(DEC, left, mod, call); \
      VIRTUAL_CASE(ADD, left, mod, call); \
      VIRTUAL_CASE(SUB, left, mod, call); \
      VIRTUAL_CASE(MUL, left, mod, call); \
      VIRTUAL_CASE(JNZ, left, mod, call); \
      VIRTUAL_CASE(JG, left, mod, call); \
      VIRTUAL_CASE(JNG, left, mod, call); \
      VIRTUAL_CASE(JE, left, mod, call); \
      VIRTUAL_CASE(CPI1, left, mod, call); \
      VIRTUAL_CASE(CPI2, left, mod, call); \
      VIRTUAL_CASE(CPI4, left, mod, call); \
      VIRTUAL_CASE(CPI8, left, mod, call); \
      VIRTUAL_CASE(PUSH, left, mod, call); \
      VIRTUAL_CASE(PUSHR, left, mod, call); \
      VIRTUAL_CASE(PUSHH, left, mod, call); \
      VIRTUAL_CASE(POP, left, mod, call); \
      VIRTUAL_CASE(JMP, left, mod, call); \
      VIRTUAL_CASE(CPB, left, mod, call); \
      VIRTUAL_CASE(LDE, left, mod, call); \
      VIRTUAL_CASE(LDB, left, mod, call); \
      VIRTUAL_CASE(LDR, left, mod, call); \
      VIRTUAL_CASE(STE, left, mod, call); \
      VIRTUAL_CASE(STB, left, mod, call); \
      VIRTUAL_CASE(STR, left, mod, call); \
      VIRTUAL_CASE(CALL, left, mod, call); \
      VIRTUAL_CASE(THROW, left, mod, call); \
      VIRTUAL_CASE(RET, left, mod, call); \
      default: left def; \
    }

    size_t Instr::size() const {
      VIRTUAL_CALL(return, const, size(), 0);
    }

    void Instr::setParam(uint64_t p) {
      ostringstream out;
      writeMBUInt(p, out);
      string str = out.str();
      copy(str.begin(), str.end(), dat);
    }

    void Instr::setParam2(int64_t p) {
      ostringstream out;
      writeMBInt(p, out);
      string str = out.str();
      copy(str.begin(), str.end(), dat);
    }

    void Instr::set2Params(uint64_t p1, uint64_t p2) {
      ostringstream out;
      writeMBUInt(p1, out);
      writeMBUInt(p2, out);
      string str = out.str();
      copy(str.begin(), str.end(), dat);
    }

    void Instr::set2Params2(uint64_t p1, int64_t p2) {
      ostringstream out;
      writeMBUInt(p1, out);
      writeMBInt(p2, out);
      string str = out.str();
      copy(str.begin(), str.end(), dat);
    }

    void Instr::set3Params(uint64_t p1, uint64_t p2, uint64_t p3) {
      ostringstream out;
      writeMBUInt(p1, out);
      writeMBUInt(p2, out);
      writeMBUInt(p3, out);
      string str = out.str();
      copy(str.begin(), str.end(), dat);
    }

    void Instr::set3Params2(uint64_t p1, uint64_t p2, int64_t p3) {
      ostringstream out;
      writeMBUInt(p1, out);
      writeMBUInt(p2, out);
      writeMBInt(p3, out);
      string str = out.str();
      copy(str.begin(), str.end(), dat);
    }

#define INSTR_ISSTREAM(in) \
    const char *buf = reinterpret_cast<const char*>(dat); \
    istringstream in(string(buf, MAX_INSTR_SIZE - 1));

    size_t Instr::size(int paramCount) const {
      INSTR_ISSTREAM(in);
      size_t size = 1;
      uint64_t val;

      for(int i = 0; i < paramCount; i++)
        size += readMBUInt(in, val);

      return size;
    }

    uint64_t Instr::getParam(int index) const {
      INSTR_ISSTREAM(in);
      uint64_t val;

      for(int i = 0; i <= index; i++)
        readMBUInt(in, val);

      return val;
    }

    int64_t Instr::getParam2(int index) const {
      INSTR_ISSTREAM(in);
      int64_t val;

      for(int i = 0; i <= index; i++)
        readMBInt(in, val);

      return val;
    }

    void Instr::assertRegExists(const ModuleBuilder &mbuilder, RegId reg) {
      mbuilder.assertRegExists(reg);
    }

    void Instr::assertRegAllocated(const ModuleBuilder &mbuilder, ProcId proc,
                                   FrameType ftype, RegId reg) {
      mbuilder.assertRegExists(reg);
      mbuilder.assertRegAllocated(proc, ftype, reg);
    }

    void Instr::assertRegHasBytes(const ModuleBuilder &mbuilder, ProcId proc,
                                  RegId reg, uint32_t bytes) {
      assertRegAllocated(mbuilder, proc, FT_REG, reg);

      VarSpec vspec;
      mbuilder.regById(reg, vspec);
      VarType vtype;
      mbuilder.varTypeById(vspec.vtype, vtype);

      if(vtype.bytes < bytes)
        throw TypeException();
    }

    void Instr::assertSameVarType(VarTypeId vtype1, VarTypeId vtype2) {
      if(vtype1 != vtype2)
        throw TypeException();
    }

    void Instr::assertSafeRefCopy(const VarSpec &from, const VarSpec &to) {
      bool nfFrom = from.flags & VFLAG_NON_FIXED_REF;
      bool nfTo = to.flags & VFLAG_NON_FIXED_REF;
      if(from.vtype != to.vtype || nfFrom != nfTo ||
         (!nfFrom && from.count != to.count))
        throw TypeException();
    }

    void Instr::assertProcCallable(const ModuleBuilder &mbuilder, ProcId proc,
                                   ProcId targetProc) {
      mbuilder.assertProcExists(targetProc);
      mbuilder.assertProcCallable(proc, targetProc);
    }

    void Instr::regSpec(const ModuleBuilder &mbuilder, ProcId proc,
			FrameType ftype, RegId reg, VarSpec &vspec) {
      assertRegAllocated(mbuilder, proc, ftype, reg);
      mbuilder.regById(reg, vspec);
    }

    void Instr::vrefSpec(const ModuleBuilder &mbuilder, ProcId proc, RegId reg,
			 uint32_t vref, VarSpec &vspec) {
      assertRegAllocated(mbuilder, proc, FT_REG, reg);
      mbuilder.regById(reg, vspec);

      VarType vtype;
      mbuilder.varTypeById(vspec.vtype, vtype);

      if(vref >= vtype.vrefs.size())
	throw RangeException();

      vspec = vtype.vrefs[vref];
    }

    void Instr::applyBeginFrame(ModuleBuilder &mbuilder, ProcId proc,
                                FrameType ftype, RegId reg) {
      mbuilder.assertRegExists(reg);
      mbuilder.applyBeginFrame(proc, ftype, reg);
    }

    void Instr::applyBeginFrame(ModuleBuilder &mbuilder, ProcId proc,
                                ptrdiff_t offset) {
      mbuilder.applyBeginFrame(proc, offset);
    }

    void Instr::applyEndFrame(ModuleBuilder &mbuilder, ProcId proc) {
      mbuilder.applyEndFrame(proc);
    }

    void Instr::applyInstrOffset(ModuleBuilder &mbuilder, ProcId proc,
                                 ptrdiff_t offset) {
      mbuilder.applyInstrOffset(proc, offset);
    }

    void Instr::applyDefault(ModuleBuilder &mbuilder, ProcId proc) {
      mbuilder.applyDefault(proc);
    }

    void Instr::assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
      VIRTUAL_CALL(, const, assertConsistency(mbuilder, proc), );
    }

    bool Instr::branches() const {
      VIRTUAL_CALL(return, const, branches(), false);
    }

    size_t Instr::branchIndex(size_t index) const {
      VIRTUAL_CALL(return, const, branchIndex(index), 0);
    }

    const char *Instr::mnemonic() const {
      VIRTUAL_CALL(return, const, dummy(opcode), "ILL");
    }

  }
}
