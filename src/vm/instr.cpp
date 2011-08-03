#include "instr.h"

namespace Ant {
  namespace VM {

    AFRMInstr::AFRMInstr(FrameId frame) {

    }

    FFRMInstr::FFRMInstr() {

    }

    MOVM8Instr::MOVM8Instr(uint64_t val, RegId to) {

    }

    MOVN8Instr::MOVN8Instr(RegId from, RegId to) {

    }

    MULInstr::MULInstr(RegId factor1, RegId factor2, RegId product) {

    }

    DECInstr::DECInstr(RegId it) {

    }

    JNZInstr::JNZInstr(int offset) {

    }

    RETInstr::RETInstr() {

    }

  }
}
