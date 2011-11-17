#include <iostream>
#include <set>
#include <sstream>

#include "../util.h"
#include "../exception.h"
#include "instr.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Constants.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LLVMContext.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Intrinsics.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "mdata.h"

namespace {

  using namespace std;
  using namespace Ant::VM;

  const char *ZTI_VAR_NAME = "_ZTIx";
  const char *THROW_FUNC_NAME = "throw";
  const char *CXA_EALLOC_FUNC_NAME = "__cxa_allocate_exception";
  const char *CXA_THROW_FUNC_NAME = "__cxa_throw";
  const char *GXX_PERS_FUNC_NAME = "__gxx_personality_v0";
  const char *DESTROY_FUNC_NAME = "ant_vm_destroy_variable";
  const char *TRACE_FUNC_NAME = "ant_vm_trace";

  inline string funcName(ProcId proc) {
    ostringstream out;
    out << 'p' << proc;
    return out.str();
  }

  inline string varName(RegId reg) {
    ostringstream out;
    out << 'v' << reg;
    return out.str();
  }

}

namespace Ant {
  namespace VM {

    using namespace std;
    using namespace llvm;

    struct Runtime::ModuleData::LLVMContext {
      struct Frame {
        FrameType ftype;
        RegId reg;
        Value *sptr;
        Value *vptr;
        size_t hindex;
        BasicBlock *ublock;
      };

      Frame *findFrame(RegId reg) {
        for(int i = frames.size() - 1; i >= 0; i--)
          if(frames[i].ftype != FT_HAND && frames[i].reg == reg)
            return &frames[i];
        return NULL;
      }
      void pushRegFrame(bool ref, RegId reg, Value *sptr, Value *vptr) {
        frames.push_back(Frame());
        frames.back().ftype = ref ? FT_REGR : FT_REGNR;
        frames.back().reg = reg;
        frames.back().sptr = sptr;
        frames.back().vptr = vptr;
        frames.back().ublock = NULL;
      }
      void pushHandFrame(size_t hindex) {
        frames.push_back(Frame());
        frames.back().ftype = FT_HAND;
        frames.back().hindex = hindex;
        frames.back().ublock = NULL;
      }

      void popFrame() { frames.pop_back(); }

      BasicBlock *branchBlock(size_t instrIndex) {
        vector<size_t>::iterator iter = lower_bound(blockIndexes.begin(),
                                                    blockIndexes.end(),
                                                    instrIndex);
        ASSERT(iter != blockIndexes.end() && *iter == instrIndex);
        return blocks[distance(blockIndexes.begin(), iter)];
      }

      ProcId proc;
      Function *func;
      size_t instrIndex, blockIndex;
      std::vector<size_t> blockIndexes;
      std::vector<llvm::BasicBlock*> blocks;
      BasicBlock *currentBlock; // sometimes != blocks[blockIndex]
      std::vector<Frame> frames;
    };

    Runtime::ModuleData::ModuleData(const UUID &id)
      : id(id), dropped(false), llvmModule(NULL), llvmFPM(NULL), llvmEE(NULL) {
      InitializeNativeTarget();
      JITExceptionHandling = true;
    }

    void Runtime::ModuleData::assertNotDropped() const {
      if(isDropped())
        throw NotFoundException();
    }

    void Runtime::ModuleData::assertUnpacked() const {
      assertNotDropped();

      if(isPacked())
        throw OperationException();
    }

    uint32_t Runtime::ModuleData::varTypeCount() const {
      assertNotDropped();
      return vtypes.size();
    }

    uint32_t Runtime::ModuleData::procTypeCount() const {
      assertNotDropped();
      return ptypes.size();
    }

    uint32_t Runtime::ModuleData::regCount() const {
      assertNotDropped();
      return regs.size();
    }
    
    uint32_t Runtime::ModuleData::procCount() const {
      assertNotDropped();
      return procs.size();
    }

    void Runtime::ModuleData::varTypeById(VarTypeId id, VarType &vtype) const {
      assertNotDropped();

      if(id >= vtypes.size())
        throw NotFoundException();

      const VarTypeData vtypeData = vtypes[id];
      vtype.bytes = vtypeData.bytes;
      vtype.vrefs.assign(vtypeData.vrefs.begin(), vtypeData.vrefs.end());
      vtype.prefs.assign(vtypeData.prefs.begin(), vtypeData.prefs.end());
    }

    void Runtime::ModuleData::procTypeById(ProcTypeId id,
                                           ProcType &ptype) const {
      assertNotDropped();

      if(id >= ptypes.size())
        throw NotFoundException();

      ptype = ptypes[id];
    }

    void Runtime::ModuleData::regById(RegId id, VarSpec &reg) const {
      assertNotDropped();

      if(id >= regs.size())
        throw NotFoundException();

      reg = regs[id];
    }

    void Runtime::ModuleData::procById(ProcId id, Proc &proc) const {
      assertNotDropped();

      if(id >= procs.size())
        throw NotFoundException();

      const ProcData procData = procs[id];
      proc.flags = procData.flags;
      proc.ptype = procData.ptype;
      proc.code.assign(procData.code.begin(), procData.code.end());
    }

    bool Runtime::ModuleData::isPacked() const {
      assertNotDropped();
      return !llvmModule;
    }

    bool Runtime::ModuleData::isDropped() const {
      return dropped;
    }

    void Runtime::ModuleData::pack() {
      assertNotDropped();

      if(llvmModule) {
        delete llvmEE;
        llvmEE = NULL;
        delete llvmFPM;
        llvmFPM = NULL;
        // llvmEE already deleted llvmModule
        llvmModule = NULL;
      }
    }

#define TYPE_INT(bits) IntegerType::get(llvmModule->getContext(), bits)
#define TYPE_PTR(type) PointerType::get(type, 0)
#define TYPE_BARR(len) ArrayType::get(TYPE_INT(8), len)
#define TYPE_PBARR(len) ArrayType::get(TYPE_PTR(TYPE_INT(8)), len)

    Type *Runtime::ModuleData::getEltLLVMType(VarTypeId vtype) const {
      vector<Type*> fields;
      if(vtypes[vtype].bytes)
        fields.push_back(TYPE_BARR(vtypes[vtype].bytes));
      if(vtypes[vtype].vrefs.size())
        fields.push_back(TYPE_PBARR(vtypes[vtype].vrefs.size()));
      if(vtypes[vtype].prefs.size())
        fields.push_back(TYPE_PBARR(vtypes[vtype].prefs.size()));
      return StructType::get(llvmModule->getContext(), fields, false);
    }

#define CF context.func
#define CB context.currentBlock

    void Runtime::ModuleData::prepareLLVMContext(LLVMContext &context) {
      bool newBlock = false;
      set<size_t> indexes;
      Instr instr;
      indexes.insert(0);
      for(size_t i = 0, j = 0; j < procs[context.proc].code.size();
          i++, j += instr.size()) {
        if(newBlock) {
          indexes.insert(i);
          newBlock = false;
        }

        instr.set(&procs[context.proc].code[j]);
        if(instr.branches()) {
          size_t bi = instr.branchIndex(i);
          if(bi != i + 1) // this line prevents an unnecessary ending block
            indexes.insert(bi);
          newBlock = true;
        }
      }
      context.blockIndexes.assign(indexes.begin(), indexes.end());
      sort(context.blockIndexes.begin(), context.blockIndexes.end());

      context.blocks.reserve(context.blockIndexes.size());
      for(size_t i = 0; i < context.blockIndexes.size(); i++)
        context.blocks.push_back(BasicBlock::Create(llvmModule->getContext(),
                                                    "", CF, 0));
      CB = context.blocks[0];

      context.pushRegFrame(false, ptypes[procs[context.proc].ptype].io, NULL,
                           CF->arg_begin());
    }

#define CONST_INT(bits, val, signed) \
  ConstantInt::get(llvmModule->getContext(), APInt(bits, val, signed))
#define BITCAST_PINT(bits, vptr, block) \
    new BitCastInst(vptr, TYPE_PTR(TYPE_INT(bits)), "", block)
#define CALL_FUNC(block, var, func, args) \
    CallInst *var = CallInst::Create(func, args, "", block); \
    var->setCallingConv(func->getCallingConv());

    void Runtime::ModuleData::emitThrowIfNot(Function *func,BasicBlock *&block,
                                             Value *cond, int64_t edValue) {
      BasicBlock *fblock = BasicBlock::Create(llvmModule->getContext(), "",
                                              func, 0);
      Function *thrw = llvmModule->getFunction(THROW_FUNC_NAME);
      vector<Value*> args(1, CONST_INT(64, edValue, true));
      Value *edptr =llvmModule->getGlobalVariable(varName(PRESET_REG_ED),true);
      edptr = BITCAST_PINT(64, edptr, fblock);
      new StoreInst(args[0], edptr, fblock);
      CALL_FUNC(fblock, call, thrw, args);
      new UnreachableInst(llvmModule->getContext(), fblock);

      BasicBlock *tblock = BasicBlock::Create(llvmModule->getContext(), "",
                                              func, 0);
      BranchInst::Create(tblock, fblock, cond, block);
      block = tblock;
    }

    Value *Runtime::ModuleData::emitSpecialPtr(BasicBlock *block, Value *vptr,
                                               SpeField sfld) {
      Value *iptr = BITCAST_PINT(64, vptr, block);
      Value *index = CONST_INT(32, (sfld == SFLD_REF_COUNT) ? -1 : -2, true);
      return GetElementPtrInst::Create(iptr, index, "", block);
    }

    Value *Runtime::ModuleData::emitElementPtr(Function *func,
                                               BasicBlock *&block, RegId reg,
                                               bool ref,Value *vptr,
                                               size_t eltc, Value *eltv) {
      bool runtimeCheck = eltv ||
        (ref && regs[reg].flags & VFLAG_NON_FIXED_REF);

      if(runtimeCheck || eltc) { // just to remove redundancy
        if(!eltv)
          eltv = CONST_INT(64, uint64_t(eltc), false);

        if(runtimeCheck) {
          Value *ecval;
          if(regs[reg].flags & VFLAG_NON_FIXED_REF)
            ecval = new LoadInst(emitSpecialPtr(block, vptr, SFLD_ELT_COUNT),
                                 "", block);
          else ecval = CONST_INT(64, uint64_t(regs[reg].count), false);

          Value *cond = new ICmpInst(*block, ICmpInst::ICMP_ULT, eltv, ecval);
          emitThrowIfNot(func, block, cond, VMECODE_RANGE);
        }
        return GetElementPtrInst::Create(vptr, eltv, "", block);
      }
      else return vptr;
    }

    Value *Runtime::ModuleData::emitFieldPtr(BasicBlock *block, Value *vptr,
                                             EltField efld, uint32_t eltc) {
      vector<Value*> indexes;
      indexes.push_back(CONST_INT(64, 0, false));

      uint64_t index;
      const StructType *st = static_cast<const StructType*>
        (static_cast<const PointerType*>(vptr->getType())->getElementType());
      switch(efld) {
        case EFLD_BYTES: index = 0; break;
        case EFLD_VREFS: index = static_cast<const ArrayType*>
          (st->getElementType(0))->getElementType()->isIntegerTy(); break;
        case EFLD_PREFS: index = st->getNumElements() - 1; break;
      };
      indexes.push_back(CONST_INT(32, index, false));

      indexes.push_back(CONST_INT(32, uint64_t(eltc), false));

      return GetElementPtrInst::Create(vptr, indexes, "", block);
    }

    Value *Runtime::ModuleData::emitRegValue(LLVMContext &context, RegId reg,
                                             bool dereferenceIfNeeded,
                                             size_t eltc, Value *eltv) {
      LLVMContext::Frame *frame = context.findFrame(reg);
      if(frame) {
        Value *vptr = frame->vptr;
        if(frame->ftype == FT_REGR) {
          if(!dereferenceIfNeeded)
            return vptr;

          vptr = new LoadInst(vptr, "", CB);
          PointerType *ptype = static_cast<PointerType*>(vptr->getType());
          Constant *null = ConstantPointerNull::get(ptype);
          Value *cond = new ICmpInst(*CB, ICmpInst::ICMP_NE, vptr, null);
          emitThrowIfNot(CF, CB, cond, VMECODE_NULL_REFERENCE);
        }

        return emitElementPtr(CF, CB, reg, frame->ftype == FT_REGR, vptr, eltc,
                              eltv);
      }
      else {
        Value *vptr = llvmModule->getGlobalVariable(varName(reg), true);
        return new BitCastInst(vptr, TYPE_PTR(getEltLLVMType(regs[reg].vtype)),
                               "", CB);
      }
    }

    template<uint8_t OP, Instruction::BinaryOps IOP, uint64_t CO>
      void Runtime::ModuleData::emitLLVMCodeUO(LLVMContext &context,
                                               const UOInstrT<OP> &instr) {
      Value *it = BITCAST_PINT(64, emitRegValue(context, instr.it()), CB);
      Value *val = new LoadInst(it, "", CB);
      Value *co = CONST_INT(64, CO, false);
      val = BinaryOperator::Create(IOP, val, co, "", CB);
      new StoreInst(val, it, CB);
    }

    template<uint8_t OP, Instruction::BinaryOps IOP>
      void Runtime::ModuleData::emitLLVMCodeBO(LLVMContext &context,
                                               const BOInstrT<OP> &instr) {
      Value *operand1 =
        BITCAST_PINT(64, emitRegValue(context, instr.operand1()), CB);
      Value *operand2 =
        BITCAST_PINT(64, emitRegValue(context, instr.operand2()), CB);
      Value *result =
        BITCAST_PINT(64, emitRegValue(context, instr.result()), CB);
      Value *val1 = new LoadInst(operand1, "", CB);
      Value *val2 = new LoadInst(operand2, "", CB);
      Value *val3 = BinaryOperator::Create(IOP, val1, val2, "", CB);
      new StoreInst(val3, result, CB);
    }

    template<uint8_t OP, ICmpInst::Predicate PR, uint64_t CO>
      void Runtime::ModuleData::emitLLVMCodeUJ(LLVMContext &context,
                                               const UJInstrT<OP> &instr) {
      Value *it = BITCAST_PINT(64, emitRegValue(context, instr.it()), CB);
      Value *val = new LoadInst(it, "", CB);
      ICmpInst* cmp = new ICmpInst(*CB, PR, val, CONST_INT(64, CO, false));
      size_t jindex = instr.branchIndex(context.instrIndex);
      BasicBlock *tblock = context.branchBlock(jindex);
      BasicBlock *fblock = context.blocks[context.blockIndex + 1];
      BranchInst::Create(tblock, fblock, cmp, CB);
    }

    template<uint8_t OP, ICmpInst::Predicate PR>
      void Runtime::ModuleData::emitLLVMCodeBJ(LLVMContext &context,
                                               const BJInstrT<OP> &instr) {
      Value *operand1 =
        BITCAST_PINT(64, emitRegValue(context, instr.operand1()), CB);
      Value *operand2 =
        BITCAST_PINT(64, emitRegValue(context, instr.operand2()), CB);
      Value *val1 = new LoadInst(operand1, "", CB);
      Value *val2 = new LoadInst(operand2, "", CB);
      ICmpInst* cmp = new ICmpInst(*CB, PR, val1, val2);
      size_t jindex = instr.branchIndex(context.instrIndex);
      BasicBlock *tblock = context.branchBlock(jindex);
      BasicBlock *fblock = context.blocks[context.blockIndex + 1];
      BranchInst::Create(tblock, fblock, cmp, CB);
    }

    template<uint8_t OP, class VAL>
      void Runtime::ModuleData::emitLLVMCodeCPI(LLVMContext &context,
                                             const CPIInstrT<OP, VAL> &instr) {
      int bits = sizeof(VAL) << 3;
      Value *to = BITCAST_PINT(bits, emitRegValue(context, instr.to()), CB);
      new StoreInst(CONST_INT(bits, uint64_t(instr.val()), false), to, CB);
    }

    Value *Runtime::ModuleData::emitZeroVariable(BasicBlock *block,
                                                 Value *vptr, Value *count) {
      vector<Value*> args(1, count);
      Value *eptr = GetElementPtrInst::Create(vptr, args, "", block);
      Value *vptri = new PtrToIntInst(vptr, TYPE_INT(64), "", block);
      Value *eptri = new PtrToIntInst(eptr, TYPE_INT(64), "", block);
      Value *len = BinaryOperator::Create(Instruction::Sub, eptri, vptri, "",
                                          block);

      vector<Type*> types;
      types.push_back(TYPE_PTR(TYPE_INT(8)));
      types.push_back(TYPE_INT(64));
      Function *ms = Intrinsic::getDeclaration(llvmModule, Intrinsic::memset,
                                               types);
      args.clear();
      args.push_back(BITCAST_PINT(8, vptr, block));
      args.push_back(CONST_INT(8, 0, false));
      args.push_back(len);
      args.push_back(CONST_INT(32, 0, false));
      args.push_back(CONST_INT(1, 0, false));
      CALL_FUNC(block, call, ms, args);
    }

#define CONST_PTR(type, ptr) \
    ConstantExpr::getCast(Instruction::IntToPtr, \
                          CONST_INT(64, uint64_t(ptr), false), \
                          TYPE_PTR(type))

    template<uint8_t OP, bool REF>
      void Runtime::ModuleData::emitLLVMCodePUSH(LLVMContext &context,
                                            const PUSHInstrT<OP, REF> &instr) {
      Function *ss = Intrinsic::getDeclaration(llvmModule,
                                               Intrinsic::stacksave);
      Value *sptr = CallInst::Create(ss, "", CB), *vptr;

      RegId reg = instr.reg();
      Type *type = getEltLLVMType(regs[reg].vtype);

      if(REF) {
        PointerType *ptype = TYPE_PTR(type);
        vptr = new AllocaInst(ptype, "", CB);
        Constant *null = ConstantPointerNull::get(ptype);
        new StoreInst(null, vptr, CB);
      }
      else {
        Value *count = CONST_INT(64, uint64_t(regs[reg].count), false);
        vptr = new AllocaInst(type, count, "", CB);
        emitZeroVariable(CB, vptr, count);
      }

      context.pushRegFrame(REF, reg, sptr, vptr);
    }

    void Runtime::ModuleData::emitLLVMCodePUSHH(LLVMContext &context,
                                                const PUSHHInstr &instr) {
      context.pushHandFrame(instr.branchIndex(context.instrIndex));
    }

    typedef void (*AntVMDestroyVariablePtr)(const vector<VarTypeData>&,
                                            const VarSpec&, Variable*);

    extern "C" void ant_vm_destroy_variable(const vector<VarTypeData> &vtypes, 
                                        const VarSpec &vspec, Variable *vptr) {
      // to be done
    }

    void Runtime::ModuleData::emitIncVarRefCount(Function *func,
                                               BasicBlock *&block, Value *vptr,
                                               const VarSpec *vspecForDec) {
      PointerType *ptype = static_cast<PointerType*>(vptr->getType());
      Constant *null = ConstantPointerNull::get(ptype);
      Value *cond = new ICmpInst(*block, ICmpInst::ICMP_NE, vptr, null);
      BasicBlock *incBlock = BasicBlock::Create(llvmModule->getContext(), "",
                                                func, 0);
      BasicBlock *endBlock = BasicBlock::Create(llvmModule->getContext(), "",
                                                func, 0);
      BranchInst::Create(incBlock, endBlock, cond, block);
      block = endBlock;

      Value *rcptr = emitSpecialPtr(incBlock, vptr, SFLD_REF_COUNT);
      Value *rcval = new LoadInst(rcptr, "", incBlock);
      Value *inc = CONST_INT(64, vspecForDec ? -1 : 1, true);
      rcval = BinaryOperator::Create(Instruction::Add, rcval, inc,"",incBlock);
      new StoreInst(rcval, rcptr, incBlock);

      if(vspecForDec) {
        cond = new ICmpInst(*incBlock, ICmpInst::ICMP_NE, rcval,
                            CONST_INT(64, 0, false));
        BasicBlock *desBlock = BasicBlock::Create(llvmModule->getContext(), "",
                                                  func, 0);
        BranchInst::Create(endBlock, desBlock, cond, incBlock);

        vptr = BITCAST_PINT(8, vptr, desBlock);
        Function *des = llvmModule->getFunction(DESTROY_FUNC_NAME);
        vector<Value*> args;
        args.push_back(CONST_PTR(TYPE_INT(8), intptr_t(&vtypes)));
        args.push_back(CONST_PTR(TYPE_INT(8), intptr_t(vspecForDec)));
        args.push_back(vptr);
        CALL_FUNC(desBlock, call, des, args);
        BranchInst::Create(endBlock, desBlock);
      }
      else BranchInst::Create(endBlock, incBlock);
    }

    void Runtime::ModuleData::emitCleanupRegFrame(Function *func,
                                                  BasicBlock *&block,RegId reg,
                                                  bool ref, Value *vptr) {
      if(ref) {
        vptr = new LoadInst(vptr, "", block);
        emitIncVarRefCount(func, block, vptr, &regs[reg]);
      }
      else for(size_t elt = 0; elt < regs[reg].count; elt++) {
          FixedArray<VarSpec> &vrefs = vtypes[regs[reg].vtype].vrefs;
          for(uint32_t vref = 0; vref < vrefs.size(); vref++) {
            vptr = emitElementPtr(func, block, reg, false, vptr, elt);
            vptr = emitFieldPtr(block, vptr, EFLD_VREFS, vref);
            Type *ty = TYPE_PTR(TYPE_PTR(getEltLLVMType(vrefs[vref].vtype)));
            vptr = new BitCastInst(vptr, ty, "", block);
            vptr = new LoadInst(vptr, "", block);
            emitIncVarRefCount(func, block, vptr, &vrefs[vref]);
          }
        }
    }

    void Runtime::ModuleData::emitLLVMCodePOP(LLVMContext &context,
                                              const POPInstr &instr) {
      LLVMContext::Frame &frame = context.frames.back();
      if(frame.ftype != FT_HAND) {
        emitCleanupRegFrame(CF, CB, frame.reg, frame.ftype == FT_REGR,
                            frame.vptr);

        Function *sr = Intrinsic::getDeclaration(llvmModule,
                                                 Intrinsic::stackrestore);
        vector<Value*> args(1, frame.sptr);
        CALL_FUNC(CB, call, sr, args);
      }

      context.popFrame();
    }

    void Runtime::ModuleData::emitLLVMCodeJMP(LLVMContext &context,
                                              const JMPInstr &instr) {
      size_t jindex = instr.branchIndex(context.instrIndex);
      BranchInst::Create(context.branchBlock(jindex), CB);
    }

#define BITCAST_PARR(bytes, vptr, block) \
    new BitCastInst(vptr, TYPE_PTR(TYPE_BARR(bytes)), "", block)

    void Runtime::ModuleData::emitLLVMCodeCPB(LLVMContext &context,
                                              const CPBInstr &instr) {
      RegId f = instr.from(), t = instr.to();
      uint32_t fbytes = vtypes[regs[f].vtype].bytes;
      uint32_t tbytes = vtypes[regs[t].vtype].bytes;
      uint32_t mbytes = fbytes < tbytes ? fbytes : tbytes;
      Value *from = BITCAST_PARR(mbytes, emitRegValue(context, f), CB);
      Value *to = BITCAST_PARR(mbytes, emitRegValue(context, t), CB);
      Value *val = new LoadInst(from, "", CB);
      new StoreInst(val, to, CB);
    }

    void Runtime::ModuleData::emitLLVMCodeLDE(LLVMContext &context,
                                              const LDEInstr &instr) {
      Value *eltptr = BITCAST_PINT(64, emitRegValue(context, instr.elt()), CB);
      Value *eltval = new LoadInst(eltptr, "", CB);
      Value *from = emitRegValue(context, instr.from(), true, 0, eltval);
      Value *val = new LoadInst(from, "", CB);
      Value *to = emitRegValue(context, instr.to());
      new StoreInst(val, to, CB);
    }

    void Runtime::ModuleData::emitLLVMCodeLDB(LLVMContext &context,
                                              const LDBInstr &instr) {
      uint32_t o = instr.offset();
      RegId f = instr.from(), t = instr.to();
      uint32_t fbytes = vtypes[regs[f].vtype].bytes - o;
      uint32_t tbytes = vtypes[regs[t].vtype].bytes;
      uint32_t mbytes = fbytes < tbytes ? fbytes : tbytes;
      Value *bptr = emitFieldPtr(CB, emitRegValue(context, f), EFLD_BYTES, o);
      Value *from = BITCAST_PARR(mbytes, bptr, CB);
      Value *to = BITCAST_PARR(mbytes, emitRegValue(context, t), CB);
      Value *val = new LoadInst(from, "", CB);
      new StoreInst(val, to, CB);
    }

    void Runtime::ModuleData::emitLLVMCodeLDR(LLVMContext &context,
                                              const LDRInstr &instr) {
      uint32_t r = instr.vref();
      RegId f = instr.from(), t = instr.to();
      const VarSpec &rvs = vtypes[regs[f].vtype].vrefs[r];
      Type *ty = TYPE_PTR(TYPE_PTR(getEltLLVMType(rvs.vtype)));
      Value *from = emitFieldPtr(CB, emitRegValue(context, f), EFLD_VREFS, r);
      from = new BitCastInst(from, ty, "", CB);
      Value *fval = new LoadInst(from, "", CB);
      emitIncVarRefCount(CF, CB, fval);
      Value *to = emitRegValue(context, t, false);
      Value *tval = new LoadInst(to, "", CB);
      emitIncVarRefCount(CF, CB, tval, &regs[t]);
      new StoreInst(fval, to, CB);
    }

    void Runtime::ModuleData::emitLLVMCodeSTE(LLVMContext &context,
                                              const STEInstr &instr) {
      Value *from = emitRegValue(context, instr.from());
      Value *eltptr = BITCAST_PINT(64, emitRegValue(context, instr.elt()), CB);
      Value *eltval = new LoadInst(eltptr, "", CB);
      Value *to = emitRegValue(context, instr.to(), true, 0, eltval);
      Value *val = new LoadInst(from, "", CB);
      new StoreInst(val, to, CB);
    }

    void Runtime::ModuleData::emitLLVMCodeSTB(LLVMContext &context,
                                              const STBInstr &instr) {
      uint32_t o = instr.offset();
      RegId f = instr.from(), t = instr.to();
      uint32_t fbytes = vtypes[regs[f].vtype].bytes;
      uint32_t tbytes = vtypes[regs[t].vtype].bytes - o;
      uint32_t mbytes = fbytes < tbytes ? fbytes : tbytes;
      Value *from = BITCAST_PARR(mbytes, emitRegValue(context, f), CB);
      Value *bptr = emitFieldPtr(CB, emitRegValue(context, t), EFLD_BYTES, o);
      Value *to = BITCAST_PARR(mbytes, bptr, CB);
      Value *val = new LoadInst(from, "", CB);
      new StoreInst(val, to, CB);
    }

    void Runtime::ModuleData::emitLLVMCodeSTR(LLVMContext &context,
                                              const STRInstr &instr) {
      uint32_t r = instr.vref();
      RegId f = instr.from(), t = instr.to();
      Value *from = emitRegValue(context, f, false);
      Value *fval = new LoadInst(from, "", CB);
      emitIncVarRefCount(CF, CB, fval);
      const VarSpec &rvs = vtypes[regs[t].vtype].vrefs[r];
      Type *ty = TYPE_PTR(TYPE_PTR(getEltLLVMType(rvs.vtype)));
      Value *to = emitFieldPtr(CB, emitRegValue(context, t), EFLD_VREFS, r);
      to = new BitCastInst(to, ty, "", CB);
      Value *tval = new LoadInst(to, "", CB);
      emitIncVarRefCount(CF, CB, tval, &rvs);
      new StoreInst(fval, to, CB);
    }

#define TYPE_LPI StructType::get(TYPE_PTR(TYPE_INT(8)), TYPE_INT(32), NULL)

    void Runtime::ModuleData::emitFuncCall(LLVMContext &context,
                                           Function *func, Value *arg) {
      vector<Value*> args(1, arg);
      if(context.frames.size() == 1) {
        CALL_FUNC(CB, call, func, args);
        return;
      }

      int fi = context.frames.size() - 1;
      BasicBlock *&ublock = context.frames[fi].ublock;
      if(!ublock) {
        BasicBlock *block=BasicBlock::Create(llvmModule->getContext(),"",CF,0);
        Function *pers = llvmModule->getFunction(GXX_PERS_FUNC_NAME);
        LandingPadInst *lp = LandingPadInst::Create(TYPE_LPI, pers,0,"",block);
        ublock = block;

        for(; fi > 1; fi--) {
          LLVMContext::Frame &frame = context.frames[fi];
          if(frame.ftype == FT_HAND)
            if(context.instrIndex >= frame.hindex)
              continue;
            else break;

          emitCleanupRegFrame(CF, block, frame.reg, frame.ftype == FT_REGR,
                              frame.vptr);
        }

        if(fi > 1) {
          lp->addClause(ConstantPointerNull::get(TYPE_PTR(TYPE_INT(8))));
          BranchInst::Create(context.branchBlock(context.frames[fi].hindex),
                             block);
        } else {
          lp->setCleanup(true);
          ResumeInst::Create(lp, block);
        }
      }

      BasicBlock *nblock =BasicBlock::Create(llvmModule->getContext(),"",CF,0);
      InvokeInst *inv = InvokeInst::Create(func, nblock, ublock, args, "", CB);
      inv->setCallingConv(func->getCallingConv());
      CB = nblock;
    }

    void Runtime::ModuleData::emitLLVMCodeCALL(LLVMContext &context,
                                               const CALLInstr &instr) {
      Function *func = llvmModule->getFunction(funcName(instr.proc()));
      emitFuncCall(context, func, context.frames.back().vptr);
    }

    void Runtime::ModuleData::emitLLVMCodeTHROW(LLVMContext &context,
                                                const THROWInstr &instr) {
      Function *thrw = llvmModule->getFunction(THROW_FUNC_NAME);
      Value *edptr =llvmModule->getGlobalVariable(varName(PRESET_REG_ED),true);
      Value *edval = new LoadInst(BITCAST_PINT(64, edptr, CB), "", CB);
      emitFuncCall(context, thrw, edval);
    }

    void Runtime::ModuleData::emitLLVMCodeRET(LLVMContext &context,
                                              const RETInstr &instr) {
      ReturnInst::Create(llvmModule->getContext(), CB);
    }

#ifdef CONFIG_DEBUG
    extern "C" void ant_vm_trace(uint64_t index, uint8_t op, void *ptr) {
      cerr << "Trace hit: "<< index << " "
           << Instr::opcodeMnemonic(OpCode(op));
      if(ptr)
        cerr << " " << ptr;
      cerr << endl;
    }

    void Runtime::ModuleData::createTraceFunc() {
      vector<Type*> argTypes;
      argTypes.push_back(TYPE_INT(64));
      argTypes.push_back(TYPE_INT(8));
      argTypes.push_back(TYPE_PTR(TYPE_INT(8)));
      Type *retType = Type::getVoidTy(llvmModule->getContext());
      FunctionType *ftype = FunctionType::get(retType, argTypes, false);

      Function* func = Function::Create(ftype, GlobalValue::ExternalLinkage,
                                        TRACE_FUNC_NAME, llvmModule);
      func->setCallingConv(CallingConv::C);

      llvmEE->addGlobalMapping(func, funcPtrToVoidPtr(&ant_vm_trace));
    }

    void Runtime::ModuleData::emitTrace(BasicBlock *block, size_t index,
                                        OpCode op, Value *ptr) {
      PointerType *ptype = TYPE_PTR(TYPE_INT(8));
      if(ptr)
        ptr = new BitCastInst(ptr, ptype, "", block);
      else ptr = ConstantPointerNull::get(ptype);

      Function *trc = llvmModule->getFunction(TRACE_FUNC_NAME);
      vector<Value*> args;
      args.push_back(CONST_INT(64, uint64_t(index), false));
      args.push_back(CONST_INT(8, uint64_t(op), false));
      args.push_back(ptr);
      CALL_FUNC(block, call, trc, args);
    }
#endif

#define UOINSTR_CASE(op, iop, co) \
    case OPCODE_##op: \
      emitLLVMCodeUO<OPCODE_##op, Instruction::iop, co>( \
        context, static_cast<UOInstrT<OPCODE_##op>&>(instr)); break;

#define BOINSTR_CASE(op, iop) \
    case OPCODE_##op: \
      emitLLVMCodeBO<OPCODE_##op, Instruction::iop>( \
        context, static_cast<BOInstrT<OPCODE_##op>&>(instr)); break;

#define CPIINSTR_CASE(op, val) \
    case OPCODE_##op: \
      emitLLVMCodeCPI<OPCODE_##op, val>( \
        context, static_cast<CPIInstrT<OPCODE_##op, val>&>(instr)); break;

#define UJINSTR_CASE(op, pr, co) \
    case OPCODE_##op: \
      emitLLVMCodeUJ<OPCODE_##op, ICmpInst::pr, co>( \
        context, static_cast<UJInstrT<OPCODE_##op>&>(instr)); break;

#define BJINSTR_CASE(op, pr) \
    case OPCODE_##op: \
      emitLLVMCodeBJ<OPCODE_##op, ICmpInst::pr>( \
        context, static_cast<BJInstrT<OPCODE_##op>&>(instr)); break;

#define PUSHINSTR_CASE(op, ref) \
    case OPCODE_##op: \
      emitLLVMCodePUSH<OPCODE_##op, ref>( \
        context, static_cast<PUSHInstrT<OPCODE_##op, ref>&>(instr)); break;

#define INSTR_CASE(op) \
    case OPCODE_##op: \
      emitLLVMCode##op(context, static_cast<op##Instr&>(instr)); break;

    void Runtime::ModuleData::emitLLVMCode(LLVMContext &context) {
      Instr instr;
      for(size_t i = 0; i < procs[context.proc].code.size();
          i += instr.size()) {
        instr.set(&procs[context.proc].code[i]);

#ifdef CONFIG_DEBUG
        emitTrace(CB, context.instrIndex, instr.opcode());
#endif
        switch(instr.opcode()) {
          UOINSTR_CASE(INC, Add, 1);
          UOINSTR_CASE(DEC, Sub, 1);
          BOINSTR_CASE(ADD, Add);
          BOINSTR_CASE(SUB, Sub);
          BOINSTR_CASE(MUL, Mul);
          UJINSTR_CASE(JNZ, ICMP_NE, 0);
          BJINSTR_CASE(JG, ICMP_SGT);
          BJINSTR_CASE(JNG, ICMP_SLE);
          BJINSTR_CASE(JE, ICMP_EQ);
          CPIINSTR_CASE(CPI1, uint8_t);
          CPIINSTR_CASE(CPI2, uint16_t);
          CPIINSTR_CASE(CPI4, uint32_t);
          CPIINSTR_CASE(CPI8, uint64_t);
          PUSHINSTR_CASE(PUSH, false);
          PUSHINSTR_CASE(PUSHR, true);
          INSTR_CASE(PUSHH);
          INSTR_CASE(POP);
          INSTR_CASE(JMP);
          INSTR_CASE(CPB);
          INSTR_CASE(LDE);
          INSTR_CASE(LDB);
          INSTR_CASE(LDR);
          INSTR_CASE(STE);
          INSTR_CASE(STB);
          INSTR_CASE(STR);
          INSTR_CASE(CALL);
          INSTR_CASE(THROW);
          INSTR_CASE(RET);
        }

        context.instrIndex++;
        size_t nextBlockIndex = context.blockIndex + 1;
        if(nextBlockIndex < context.blocks.size() &&
           context.blockIndexes[nextBlockIndex] == context.instrIndex) {
          if(!CB->getTerminator())
            BranchInst::Create(context.blocks[nextBlockIndex], CB);
          CB = context.blocks[++context.blockIndex];
        }
      }

      if(!context.blocks.back()->getTerminator())
        ReturnInst::Create(llvmModule->getContext(), context.blocks.back());
    }

    extern "C" void *_ZTIx;

    void Runtime::ModuleData::createZTIVar() {
      GlobalValue *zti = new GlobalVariable(*llvmModule, TYPE_PTR(TYPE_INT(8)),
                                            true, GlobalValue::ExternalLinkage,
                                            0, ZTI_VAR_NAME);
      llvmEE->addGlobalMapping(zti, &_ZTIx);
    }

    void Runtime::ModuleData::createLLVMPVars() {
      createZTIVar();

      for(RegId reg = 0; reg < regs.size(); reg++)
        if(regs[reg].flags & VFLAG_TOP_LEVEL_REG) {
          Type *type = getEltLLVMType(regs[reg].vtype);
          type = ArrayType::get(type, regs[reg].count);
          // bool threadLocal = regs[reg].flags & VFLAG_THREAD_LOCAL_REG;
          bool threadLocal = false; // BUG in LLVM JIT

          GlobalVariable *gvar =
            new GlobalVariable(*llvmModule, type, false,
                               GlobalValue::InternalLinkage,
                               ConstantAggregateZero::get(type), varName(reg),
                               NULL, threadLocal);

          if(regs[reg].flags & VFLAG_NON_FIXED_REF) {
            new GlobalVariable(*llvmModule, TYPE_INT(64), false,
                               GlobalValue::InternalLinkage,
                               CONST_INT(64, uint64_t(regs[reg].count), true),
                               "", gvar, threadLocal);
          }
        }
    }

    extern "C" void *__cxa_allocate_exception(uint64_t);

    void Runtime::ModuleData::createCXAEAllocFunc() {
      vector<Type*> argTypes(1, TYPE_INT(64));
      Type *retType = TYPE_PTR(TYPE_INT(8));
      FunctionType *ftype = FunctionType::get(retType, argTypes, false);

      Function* func = Function::Create(ftype, GlobalValue::ExternalLinkage,
                                        CXA_EALLOC_FUNC_NAME, llvmModule);
      func->setCallingConv(CallingConv::C);

      llvmEE->addGlobalMapping(func,
                               funcPtrToVoidPtr(&__cxa_allocate_exception));
    }

    extern "C" void __cxa_throw(void*, void*, void*);

    void Runtime::ModuleData::createCXAThrowFunc() {
      vector<Type*> argTypes(3, TYPE_PTR(TYPE_INT(8)));
      Type *retType = Type::getVoidTy(llvmModule->getContext());
      FunctionType *ftype = FunctionType::get(retType, argTypes, false);

      Function* func = Function::Create(ftype, GlobalValue::ExternalLinkage,
                                        CXA_THROW_FUNC_NAME, llvmModule);
      func->setCallingConv(CallingConv::C);

      llvmEE->addGlobalMapping(func, funcPtrToVoidPtr(&__cxa_throw));
    }

    extern "C" void __gxx_personality_v0(...);

    void Runtime::ModuleData::createGXXPersFunc() {
      vector<Type*> argTypes;
      FunctionType *ftype = FunctionType::get(TYPE_INT(32), argTypes, true);

      Function* func = Function::Create(ftype, GlobalValue::ExternalLinkage,
                                        GXX_PERS_FUNC_NAME, llvmModule);
      func->setCallingConv(CallingConv::C);

      llvmEE->addGlobalMapping(func, funcPtrToVoidPtr(&__gxx_personality_v0));
    }

    void Runtime::ModuleData::createThrowFunc() {
      vector<Type*> argTypes;
      argTypes.push_back(TYPE_INT(64));
      Type *retType = Type::getVoidTy(llvmModule->getContext());
      FunctionType *ftype = FunctionType::get(retType, argTypes, false);

      Function *func = Function::Create(ftype, GlobalValue::InternalLinkage,
                                        THROW_FUNC_NAME, llvmModule);
      func->setCallingConv(CallingConv::Fast);

      BasicBlock *block = BasicBlock::Create(llvmModule->getContext(), "",
                                             func, 0);

      Function *cxa_ealloc = llvmModule->getFunction(CXA_EALLOC_FUNC_NAME);
      vector<Value*> args(1, CONST_INT(64, 8, false));
      CALL_FUNC(block, evptr, cxa_ealloc, args);

      Value *eptr = BITCAST_PINT(64, evptr, block);
      new StoreInst(func->arg_begin(), eptr, block);

      PointerType *vptrType = TYPE_PTR(TYPE_INT(8));
      Value *zti = llvmModule->getGlobalVariable(ZTI_VAR_NAME);
      zti = new BitCastInst(zti, vptrType, "", block);
      ConstantPointerNull *null = ConstantPointerNull::get(vptrType);

      Function *cxa_throw = llvmModule->getFunction(CXA_THROW_FUNC_NAME);
      args.clear();
      args.push_back(evptr);
      args.push_back(zti);
      args.push_back(null);
      CALL_FUNC(block, call, cxa_throw, args);

      new UnreachableInst(llvmModule->getContext(), block);

      llvmFPM->run(*func);
    }

    void Runtime::ModuleData::createDestroyFunc() {
      Type *vptrType = TYPE_PTR(TYPE_INT(8));
      vector<Type*> argTypes;
      argTypes.push_back(vptrType);
      argTypes.push_back(vptrType);
      argTypes.push_back(vptrType);
      Type *retType = Type::getVoidTy(llvmModule->getContext());
      FunctionType *ftype = FunctionType::get(retType, argTypes, false);

      Function* func = Function::Create(ftype, GlobalValue::ExternalLinkage,
                                        DESTROY_FUNC_NAME, llvmModule);
      func->setCallingConv(CallingConv::C);

      llvmEE->addGlobalMapping(func,
                               funcPtrToVoidPtr(&ant_vm_destroy_variable));
    }

    void Runtime::ModuleData::createLLVMFuncs() {
#ifdef CONFIG_DEBUG
      createTraceFunc();
#endif
      createCXAEAllocFunc();  
      createCXAThrowFunc();
      createGXXPersFunc();
      createThrowFunc();
      createDestroyFunc();

      for(ProcId proc = 0; proc < procs.size(); proc++) {
        vector<Type*> argTypes;
        RegId io = ptypes[procs[proc].ptype].io;
        argTypes.push_back(TYPE_PTR(getEltLLVMType(regs[io].vtype)));
        Type *voidType = Type::getVoidTy(llvmModule->getContext());
        FunctionType *ftype = FunctionType::get(voidType, argTypes, false);

        bool external = procs[proc].flags & PFLAG_EXTERNAL;
        GlobalValue::LinkageTypes link = external ?
          GlobalValue::ExternalLinkage : GlobalValue::InternalLinkage;

        Function *func = Function::Create(ftype, link, funcName(proc),
                                          llvmModule);
        func->setCallingConv(external ? CallingConv::C : CallingConv::Fast);
      }

      for(ProcId proc = 0; proc < procs.size(); proc++) {
        Function *func = llvmModule->getFunction(funcName(proc));

        LLVMContext context = { proc, func, 0, 0 };
        prepareLLVMContext(context);
        emitLLVMCode(context);

        llvmFPM->run(*func);
      }
    }

    void Runtime::ModuleData::prepareLLVMFPM() {
      llvmFPM->add(new TargetData(*llvmEE->getTargetData()));
      // add passes here
      llvmFPM->doInitialization();
    }

    void Runtime::ModuleData::unpack() {
      assertNotDropped();

      if(isPacked())
        try {
          string err, idStr = id.str();
          llvmModule = new llvm::Module(idStr, getGlobalContext());
          llvmFPM = new FunctionPassManager(llvmModule);
          llvmEE = EngineBuilder(llvmModule).setErrorStr(&err).create();
          if(!llvmEE) {
#ifdef CONFIG_DEBUG
            cerr << endl << "Cannot load JIT (" << err << ")" << endl << endl;
#endif
            throw EnvironmentException();
          }

          prepareLLVMFPM();
          createLLVMPVars();
          createLLVMFuncs();

#ifdef CONFIG_DEBUG
          cerr << endl;
          llvmModule->dump();
          cerr << endl;
          if(verifyModule(*llvmModule, PrintMessageAction))
            throw BugException();
#endif
        }
        catch(...) { pack(); throw; }
    }

    void Runtime::ModuleData::drop() {
      assertNotDropped();

      pack();

      vtypes.clear();
      vrefs.clear();
      prefs.clear();
      regs.clear();
      procs.clear();
      code.clear();

      dropped = true;
    }

    void Runtime::ModuleData::callProc(ProcId proc, Variable &io) {
      assertUnpacked();

      Function *func = llvmModule->getFunction(funcName(proc));
      if(!func)
        throw NotFoundException();

      void *vPtr = llvmEE->getPointerToFunction(func);
      uintptr_t uPtr = reinterpret_cast<uintptr_t>(vPtr);

      try { reinterpret_cast<void (*)(Variable&)>(uPtr)(io); }
      catch(int64_t) { throw RuntimeException(); }
    }

    void Runtime::ModuleData::take(ModuleData& moduleData) {
      vtypes.swap(moduleData.vtypes);
      ptypes.swap(moduleData.ptypes);
      vrefs.swap(moduleData.vrefs);
      prefs.swap(moduleData.prefs);
      regs.swap(moduleData.regs);
      procs.swap(moduleData.procs);
      code.swap(moduleData.code);
    }

  }
}
