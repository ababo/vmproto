#include <sstream>
#include <memory.h>

#include "instr.h"
#include "utils.h"
#include "../common/exception.h"

namespace Ant {
  namespace VM {

    using namespace std;

    Instr::Instr(VMCode code) {
      size_t size = reinterpret_cast<const Instr*>(code)->size();
      memcpy(dat, code, size);
    }

    void Instr::setParam(uint64_t p) {
      ostringstream out;
      writeMultibyteInteger(p, out);
      memcpy(dat, out.str().data(), out.str().size());
    }

    void Instr::set2Params(uint64_t p1, uint64_t p2) {
      ostringstream out;
      writeMultibyteInteger(p1, out);
      writeMultibyteInteger(p2, out);
      memcpy(dat, out.str().data(), out.str().size());
    }

    void Instr::set3Params(uint64_t p1, uint64_t p2, uint64_t p3) {
      ostringstream out;
      writeMultibyteInteger(p1, out);
      writeMultibyteInteger(p2, out);
      writeMultibyteInteger(p3, out);
      memcpy(dat, out.str().data(), out.str().size());
    }

    size_t Instr::size(int paramCount) const {
      const char *buf = reinterpret_cast<const char*>(dat + 1);
      istringstream in(string(buf, MAX_INSTR_SIZE - 1));
      size_t size = 1;
      uint64_t val;

      for(int i = 0; i < paramCount; i++)
        size += readMultibyteInteger(in, val);

      return size;
    }

    size_t Instr::size() const {
      switch(op) {
      case OPCODE_AST: return static_cast<const ASTInstr&>(*this).size();
      case OPCODE_FST: return static_cast<const FSTInstr&>(*this).size();
      case OPCODE_MOVM8: return static_cast<const MOVM8Instr&>(*this).size();
      case OPCODE_MOVN8: return static_cast<const MOVN8Instr&>(*this).size();
      case OPCODE_MUL: return static_cast<const MULInstr&>(*this).size();
      case OPCODE_DEC: return static_cast<const DECInstr&>(*this).size();
      case OPCODE_JNZ: return static_cast<const JNZInstr&>(*this).size();
      case OPCODE_RET: return static_cast<const RETInstr&>(*this).size();
      default: return 0;
      };
    }

    inline RegId Instr::assertRegId(RegId reg) {
      if(reg > 0x3FFF)
        throw Ant::Common::OutOfRangeException();
      return reg;
    }

    inline int Instr::assertInstrOffset(int offset) {
      // ?
      return offset;
    }

    ASTInstr::ASTInstr(RegId reg) {
      op = OPCODE_AST;
      setParam(assertRegId(reg));
    }

    FSTInstr::FSTInstr() {
      op = OPCODE_FST;
    }

    MOVM8Instr::MOVM8Instr(uint64_t val, RegId to) {
      op = OPCODE_MOVM8;
      set2Params(val, assertRegId(to));
    }

    MOVN8Instr::MOVN8Instr(RegId from, RegId to) {
      op = OPCODE_MOVN8;
      set2Params(assertRegId(from), assertRegId(to));
    }

    MULInstr::MULInstr(RegId factor1, RegId factor2, RegId product) {
      op = OPCODE_MUL;
      set3Params(assertRegId(factor1),
                 assertRegId(factor2),
                 assertRegId(product));
    }

    DECInstr::DECInstr(RegId it) {
      op = OPCODE_DEC;
      setParam(assertRegId(it));
    }

    JNZInstr::JNZInstr(int offset) {
      op = OPCODE_JNZ;
      setParam(assertInstrOffset(offset));
    }

    RETInstr::RETInstr() {
      op = OPCODE_RET;
    }

  }
}
