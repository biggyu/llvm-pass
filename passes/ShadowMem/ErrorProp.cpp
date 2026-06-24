#include "ErrorProp.h"
#include "DebugCheck.h"
#include "llvm/IR/IRBuilder.h"
// #include "llvm/IR/Intrinsics.h"
using namespace llvm;

bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &DSLMap) {

    if (!II->getType()->isDoubleTy() && !II->getType()->isFloatTy()) {
        return false;
    }
    Value *arg0 = II->getArgOperand(0);
    DSLValues arg0_dsl = getDSL(arg0, rt, DSLMap);
    Value *arg0_err = arg0_dsl.error;
    IRBuilder<> AfterII(II->getNextNode());
    //TODO: Exception Handling(arg0 < 0)
    if (II->getIntrinsicID() == Intrinsic::sqrt) {
        Value *x = II;
        Value *negVal = AfterII.CreateFNeg(x, "sqrt.negval");
        Value *fma = AfterII.CreateIntrinsic(
            Intrinsic::fma,
            {x->getType()},
            {negVal, x, arg0},
            nullptr,
            "sqrt.fma"
        );
        Value *num = AfterII.CreateFAdd(arg0_err, fma, "sqrt.num");
        Value *two = ConstantFP::get(x->getType(), 2.0);
        Value *den = AfterII.CreateFMul(two, x, "sqrt.den");
        Value *dx = AfterII.CreateFDiv(num, den, "sqrt.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Sqrt, rt);
        }
        // Value *fmt = AfterII.CreateGlobalStringPtr("sqrt_II: x=%f, dx=%e\n");
        // AfterII.CreateCall(rt.Printf, {fmt, x, dx});
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::sin) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropSinDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropSinFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "sin.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "sin.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Sin, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::cos) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropCosDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropCosFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "cos.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "cos.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Cos, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::tan) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropTanDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropTanFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "tan.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "tan.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Tan, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::asin) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropAsinDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropAsinFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "asin.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "asin.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Asin, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::acos) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropAcosDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropAcosFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "acos.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "acos.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Acos, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::atan) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropAtanDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropAtanFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "atan.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "atan.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Unknown, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::exp) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropExpDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropExpFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "Exp.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "Exp.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Exp, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::pow) {
        Value *arg1 = II->getArgOperand(1);
        DSLValues arg1_dsl = getDSL(arg1, rt, DSLMap);
        Value *arg1_err = arg1_dsl.error;
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropPowDError, {arg0, arg0_err, arg1, arg1_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropPowFError, {arg0, arg0_err, arg1, arg1_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "Pow.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "Pow.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg1_dsl, x_dsl, II, FpOp::Pow, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::log) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropLogDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropLogFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "Log.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "Log.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Log, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::fabs) {
        Value *ret;
        if (II->getType()->isDoubleTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropFabsDError, {arg0, arg0_err});
        }
        else if (II->getType()->isFloatTy()) {
            ret = AfterII.CreateCall(rt_mpfr.PropFabsFError, {arg0, arg0_err});
        }
        Value *x = AfterII.CreateExtractValue(ret, {0}, "fabsf.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "fabs.err");

        DSLValues x_dsl = makeDSL(x, dx, rt);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Unknown, rt);
        }
        return true;
    }
    else {
        return false;
    }
}

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &DSLMap) {

    if (!CI->getType()->isDoubleTy() && !CI->getType()->isFloatTy()) {
        return false;
    }
    Value *arg0 = CI->getArgOperand(0);
    DSLValues arg0_dsl = getDSL(arg0, rt, DSLMap);
    Value *arg0_err = arg0_dsl.error;
    IRBuilder<> AfterCI(CI->getNextNode());
    if (Function *Callee = CI->getCalledFunction()) {
        StringRef N = Callee->getName();
        //TODO: Exception Handling(arg0 < 0)
        if (N.contains("sqrt")) {
            Value *x = CI;
            Value *negVal = AfterCI.CreateFNeg(x, "sqrt.negval");
            Value *fma = AfterCI.CreateIntrinsic(
                Intrinsic::fma,
                {x->getType()},
                {negVal, x, arg0},
                nullptr,
                "sqrt.fma"
            );
            Value *num = AfterCI.CreateFAdd(arg0_err, fma, "sqrt.num");
            Value *two = ConstantFP::get(x->getType(), 2.0);
            Value *den = AfterCI.CreateFMul(two, x, "sqrt.den");
            Value *dx = AfterCI.CreateFDiv(num, den, "sqrt.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            // Value *fmt = AfterCI.CreateGlobalStringPtr("sqrt_CI: x=%f, dx=%e\n");
            // AfterCI.CreateCall(rt.Printf, {fm,S x, dx});
            // AfterCI.CreateCall(rt.Printf,x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Sqrt, rt);
            }
        }
        else if (N == "sin") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropSinDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "sin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "sin.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Sin, rt);
            }
            llvm::errs() << "CI::sin\n";
        }
        else if (N == "sinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropSinFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "sinf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "sinf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Sin, rt);
            }
        }
        else if (N == "cos") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropCosDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "cos.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "cos.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Cos, rt);
            }
            llvm::errs() << "cos\n";
        }
        else if (N == "cosf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropCosFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "cosf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "cosf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Cos, rt);
            }
        }
        else if (N == "tan") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropTanDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "tan.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "tan.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Tan, rt);
            }
        }
        else if (N == "tanf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropTanFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "tanf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "tanf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Tan, rt);
            }
        }
        else if (N == "asin") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAsinDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "asin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "asin.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Asin, rt);
            }
        }
        else if (N == "asinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAsinFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "asinf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "asinf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Asin, rt);
            }
        }
        else if (N == "acos") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAcosDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "acos.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "acos.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Acos, rt);
            }
        }
        else if (N == "acosf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAcosFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "acosf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "acosf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Acos, rt);
            }
        }
        else if (N == "atan") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAtanDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "atan.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "atan.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Unknown, rt);
            }
        }
        else if (N == "atanf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAtanFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "atanf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "atanf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Unknown, rt);
            }
        }
        else if (N == "log") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropLogDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "log.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "log.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Log, rt);
            }
        }
        else if (N == "logf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropLogFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "logf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "logf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Log, rt);
            }
        }
        else if (N == "exp") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "exp.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "exp.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Exp, rt);
            }
            llvm::errs() << "CI::exp\n";
        }
        else if (N == "expf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpFError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "expf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "expf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Exp, rt);
            }
        }
        else if (N == "pow") {
            Value *arg1 = CI->getArgOperand(1);
            DSLValues arg1_dsl = getDSL(arg1, rt, DSLMap);
            Value *arg1_err = arg1_dsl.error;
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropPowDError, {arg0, arg0_err, arg1, arg1_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "pow.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "pow.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg1_dsl, x_dsl, CI, FpOp::Pow, rt);
                // if (resolveFpOp(CI, opcode)) {
                //     insertCheckError(AfterCI, arg0_dsl, arg1_dsl, x_dsl, CI, opcode, rt);
                // }
            }
        }
        else if (N == "powf") {
            Value *arg1 = CI->getArgOperand(1);
            DSLValues arg1_dsl = getDSL(arg1, rt, DSLMap);
            Value *arg1_err = arg1_dsl.error;
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropPowFError, {arg0, arg0_err, arg1, arg1_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "powf.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "powf.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg1_dsl, x_dsl, CI, FpOp::Pow, rt);
                // if (resolveFpOp(CI, opcode)) {
                //     insertCheckError(AfterCI, arg0_dsl, arg1_dsl, x_dsl, CI, opcode, rt);
                // }
            }
        }
        else {
            return false;
        }
    }
    return false;
}

bool handleBinary(Instruction *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return false;
    }
    Value *opr0 = BO->getOperand(0);
    Value *opr1 = BO->getOperand(1);
    DSLValues opr0_dsl = getDSL(opr0, rt, DSLMap);
    DSLValues opr1_dsl = getDSL(opr1, rt, DSLMap);
    Value *opr0_err = opr0_dsl.error;
    Value *opr1_err = opr1_dsl.error;
    // Value *opr0_err = getError(opr0, rt.ZeroF, rt.ZeroD, DSLMap);
    // Value *opr1_err = getError(opr1, rt.ZeroF, rt.ZeroD, DSLMap);
    // FpOp opcode;
    IRBuilder<> AfterBO(BO->getNextNode());
    switch (BO->getOpcode()) {
        case Instruction::FAdd: {
            Value *x = BO;
            Value *bp = AfterBO.CreateFSub(x, opr0, "fadd.bp");
            Value *ap = AfterBO.CreateFSub(x, bp, "fadd.ap");
            Value *da = AfterBO.CreateFSub(opr0, ap, "fadd.da");
            Value *db = AfterBO.CreateFSub(opr1, bp, "fadd.db");
            Value *dab = AfterBO.CreateFAdd(db, da, "fadd.dab");
            Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fadd.tmp");
            Value *dx = AfterBO.CreateFAdd(opr1_err, tmp, "fadd.err");

            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Add, rt);
            }
            return true;
        }
        case Instruction::FSub: {
            Value *x = BO;
            Value *bp = AfterBO.CreateFSub(x, opr0, "fsub.bp");
            Value *ap = AfterBO.CreateFSub(x, bp, "fsub.ap");
            Value *da = AfterBO.CreateFSub(opr0, ap, "fsub.da");
            Value *db = AfterBO.CreateFAdd(opr1, bp, "fsub.db");
            Value *dab = AfterBO.CreateFSub(da, db, "fsub.dab");
            Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fsub.tmp");
            Value *dx = AfterBO.CreateFSub(tmp, opr1_err, "fsub.err");
            
            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Sub, rt);
            }
            return true;
        }
        case Instruction::FMul: {
            Value *x = BO;
            Value *negVal = AfterBO.CreateFNeg(x, "fmul.negval");
            Value *fma = AfterBO.CreateIntrinsic(
                Intrinsic::fma,
                {opr0->getType()},
                {opr0, opr1, negVal},
                nullptr,
                "mul.fma"
            );
            Value *adb = AfterBO.CreateFMul(opr0, opr1_err, "fmul.adb");
            Value *tmp = AfterBO.CreateFAdd(fma, adb, "fmul.tmp");
            Value *bda = AfterBO.CreateFMul(opr1, opr0_err, "fmul.bda");
            Value *dx = AfterBO.CreateFAdd(tmp, bda, "fmul.err");
            
            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Mul, rt);
                // if (resolveFpOp(BO, opcode)) {
                //     insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, opcode, rt);
                // }
            }
            return true;
        }
        case Instruction::FDiv: {
            Value *x = BO;
            Value *invopr0 = AfterBO.CreateFNeg(opr0, "fdiv.invopr0");
            Value *fma = AfterBO.CreateIntrinsic(
                Intrinsic::fma,
                {invopr0->getType()},
                {x, opr1, invopr0},
                nullptr,
                "fdiv.fma"
            );
            Value *da = AfterBO.CreateFSub(opr0_err, fma);
            Value *invval = AfterBO.CreateFNeg(x, "fdiv.invval");
            Value *numer = AfterBO.CreateIntrinsic(
                Intrinsic::fma,
                {invval->getType()},
                {invval, opr1_err, da},
                nullptr,
                "fdiv.numer"
            );
            Value *denom = AfterBO.CreateFAdd(opr1, opr1_err, "fdiv.denom");
            Value *dx = AfterBO.CreateFDiv(numer, denom, "fdiv.err");
            
            DSLValues x_dsl = makeDSL(x, dx, rt);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Div, rt);
                // if (resolveFpOp(BO, opcode)) {
                //     insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, opcode, rt);
                // }
            }
            return true;
        }
        default:
            return false;
    }
}