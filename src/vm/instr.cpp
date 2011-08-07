#include <sstream>

#include "../common/exception.h"
#include "instr.h"
#include "utils.h"

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace Ant::Common;

    void Instr::set(VMCode code) {
      size_t size = reinterpret_cast<const Instr*>(code)->size();
      
      if(!size)
        throw EncodingException();

      copy(code, code + size, &op);
    }

    size_t Instr::size() const {
      switch(op) {
      case OPCODE_AST: return static_cast<const ASTInstr&>(*this).size();
      case OPCODE_FST: return static_cast<const FSTInstr&>(*this).size();
      case OPCODE_MOVM8: return static_cast<const MOVM8Instr&>(*this).size();
      case OPCODE_MOVN8: return static_cast<const MOVN8Instr&>(*this).size();
      case OPCODE_UMUL: return static_cast<const UMULInstr&>(*this).size();
      case OPCODE_DEC: return static_cast<const DECInstr&>(*this).size();
      case OPCODE_JNZ: return static_cast<const JNZInstr&>(*this).size();
      case OPCODE_RET: return static_cast<const RETInstr&>(*this).size();
      default: return 0;
      };
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

    inline RegId Instr::assertRegId(RegId reg) {
      if(reg < RESERVED_REGS_COUNT) // temporarily
        throw ArgumentException();
      if(reg > MB_UINT_MAX(2))
        throw RangeException();
      return reg;
    }

    inline int Instr::assertInstrOffset(int offset) {
      if(offset < MB_INT_MIN(1) || offset > MB_INT_MAX(1))
        throw RangeException();
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

    UMULInstr::UMULInstr(RegId factor1, RegId factor2, RegId product) {
      op = OPCODE_UMUL;
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
      setParam2(assertInstrOffset(offset));
    }

    RETInstr::RETInstr() {
      op = OPCODE_RET;
    }

  }
}
