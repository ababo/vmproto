#include <sstream>

#include "../exception.h"
#include "instr.h"
#include "mbuilder.h"
#include "utils.h"

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
    case OPCODE_##op: left static_cast<mod op##Instr&>(*this).call; break;

#define VIRTUAL_CALL(left, mod, call, def) \
    switch(op) { \
      VIRTUAL_CASE(INC, left, mod, call); \
      VIRTUAL_CASE(DEC, left, mod, call); \
      VIRTUAL_CASE(ADD, left, mod, call); \
      VIRTUAL_CASE(SUB, left, mod, call); \
      VIRTUAL_CASE(MUL, left, mod, call); \
      VIRTUAL_CASE(AST, left, mod, call); \
      VIRTUAL_CASE(FST, left, mod, call); \
      VIRTUAL_CASE(MOVM8, left, mod, call); \
      VIRTUAL_CASE(MOVN8, left, mod, call); \
      VIRTUAL_CASE(JNZ, left, mod, call); \
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

    RegId Instr::assertRegExists(const ModuleBuilder &mbuilder, RegId reg) {
      return mbuilder.assertRegExists(reg);
    }

    RegId Instr::assertRegAllocated(const ModuleBuilder &mbuilder, ProcId proc,
                                    RegId reg) {
      mbuilder.assertRegExists(reg);
      return mbuilder.assertRegAllocated(proc, reg);
    }

    RegId Instr::assertRegHasBytes(const ModuleBuilder &mbuilder, ProcId proc,
                                   size_t minBytes, RegId reg) {
      assertRegAllocated(mbuilder, proc, reg);

      Reg r;
      mbuilder.regById(reg, r);
      VarType vtype;
      mbuilder.varTypeById(r.vtype, vtype);

      if(vtype.bytes < minBytes)
        throw TypeException();

      return reg;
    }

    void Instr::applyStackAlloc(ModuleBuilder &mbuilder, ProcId proc,
                                RegId reg) {
      mbuilder.applyStackAlloc(proc, reg);
    }

    void Instr::applyStackFree(ModuleBuilder &mbuilder, ProcId proc) {
      mbuilder.applyStackFree(proc);
    }

    void Instr::applyInstrOffset(ModuleBuilder &mbuilder, ProcId proc,
                                 int offset) {
      mbuilder.applyInstrOffset(proc, offset);
    }

    void Instr::applyDefault(ModuleBuilder &mbuilder, ProcId proc) {
      mbuilder.applyDefault(proc);
    }

    void Instr::assertConsistency(ModuleBuilder &mbuilder, ProcId proc) const {
      VIRTUAL_CALL(, const, assertConsistency(mbuilder, proc), );
    }

    bool Instr::breaks() const {
      VIRTUAL_CALL(return, const, breaks(), false);
    }

    bool Instr::jumps() const {
      VIRTUAL_CALL(return, const, jumps(), false);
    }

    size_t Instr::jumpIndex(size_t index) const {
      VIRTUAL_CALL(return, const, jumpIndex(index), 0);
    }

  }
}
