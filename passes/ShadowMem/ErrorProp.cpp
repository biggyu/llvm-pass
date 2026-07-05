#include "ErrorProp.h"
#include "DebugCheck.h"
#include "llvm/IR/IRBuilder.h"
// #include "llvm/IR/Intrinsics.h"
using namespace llvm;

static Value* getError(Value *v, Constant *ZeroD, 
                        DenseMap<const Value*, Value*> &ErrorMap) {
    auto it = ErrorMap.find(v);
    if (it != ErrorMap.end()) {
        return it->second;
    }
    return ZeroD;
    // if (v->getType()->isDoubleTy()) {
    //     return ZeroD;
    // }
    // if (v->getType()->isFloatTy()) {
    //     return ZeroF;
    // }
    // return nullptr;
}

bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap) {

    if (!II->getType()->isDoubleTy() && !II->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterII(II->getNextNode());
    bool isFloat = II->getType()->isFloatTy();
    Value *arg0_org = II->getArgOperand(0);
    Value *arg0 = isFloat ? AfterII.CreateFPExt(arg0_org, rt.DoubleTy, "II.arg0")   : arg0_org;

    Value *arg0_err = getError(arg0_org, rt.ZeroD, ErrorMap);
    //TODO: Exception Handling(arg0 < 0)
    if (II->getIntrinsicID() == Intrinsic::sqrt) {
        Value *x_org = II;
        Value *x = isFloat ? AfterII.CreateFPExt(x_org, rt.DoubleTy, "II.x")        : x_org;
        Value *negVal = AfterII.CreateFNeg(x, "sqrt.negval");
        Value *fma = AfterII.CreateIntrinsic(
            Intrinsic::fma,
            {rt.DoubleTy},
            {negVal, x, arg0},
            nullptr,
            "sqrt.fma"
        );
        Value *num = AfterII.CreateFAdd(arg0_err, fma, "sqrt.num");
        Value *two = ConstantFP::get(x->getType(), 2.0);
        Value *den = AfterII.CreateFMul(two, x, "sqrt.den");
        Value *dx = AfterII.CreateFDiv(num, den, "sqrt.err");
        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx, II, rt);
        }
        // Value *fmt = AfterII.CreateGlobalStringPtr("sqrt_II: x=%f, dx=%e\n");
        // AfterII.CreateCall(rt.Printf, {fmt, x, dx});
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::sin) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropSinDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "sin.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "sin.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::cos) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropCosDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "cos.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "cos.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::tan) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropTanDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "tan.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "tan.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::asin) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropAsinDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "asin.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "asin.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::acos) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropAcosDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "acos.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "acos.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::atan) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropAtanDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "atan.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "atan.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::exp) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropExpDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "exp.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "exp.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::pow) {
        Value *arg1_org = II->getArgOperand(1);
        Value *arg1 = isFloat ? AfterII.CreateFPExt(arg1_org, rt.DoubleTy, "II.arg1")   : arg1_org;
        Value *arg1_err = getError(arg1_org, rt.ZeroD, ErrorMap);
        Value *ret = AfterII.CreateCall(rt_mpfr.PropPowDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "pow.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "pow.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::log) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropLogDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "log.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "log.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::fabs) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropFabsDError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "fabs.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "fabs.err");

        ErrorMap[II] = dx;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, x, dx,II, rt);
        }
        // if (II->getType()->isDoubleTy()) {
        //     Value *ret = AfterII.CreateCall(rt_mpfr.PropFabsDError, {arg0, arg0_err});
        //     Value *x = AfterII.CreateExtractValue(ret, {0}, "fabs.val");
        //     Value *dx = AfterII.CreateExtractValue(ret, {1}, "fabs.err");

        //     ErrorMap[II] = dx;
        // }
        // else if (II->getType()->isFloatTy()) {
        //     Value *ret = AfterII.CreateCall(rt_mpfr.PropFabsFError, {arg0, arg0_err});
        //     // Value *x = AfterII.CreateExtractValue(ret, {0}, "fabsf.val");
        //     Value *dx = AfterII.CreateExtractValue(ret, {1}, "fabsf.err");

        //     ErrorMap[II] = dx;
        // }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::atan) {
        llvm::errs() << "II::atan\n";
        return true;
    }
    else {
        return false;
    }
}

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap) {

    if (!CI->getType()->isDoubleTy() && !CI->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterCI(CI->getNextNode());
    bool isFloat = CI->getType()->isFloatTy();
    Value *arg0_org = CI->getArgOperand(0);
    Value *arg0 = isFloat ? AfterCI.CreateFPExt(arg0_org, rt.DoubleTy, "CI.arg0")   : arg0_org;
    
    Value *arg0_err = getError(arg0_org, rt.ZeroD, ErrorMap);
    if (Function *Callee = CI->getCalledFunction()) {
        StringRef N = Callee->getName();
        //TODO: Exception Handling(arg0 < 0)
        if (N.contains("sqrt")) {
            Value *x_org = CI;
            Value *x = isFloat ? AfterCI.CreateFPExt(x_org, rt.DoubleTy, "CI.x")    : x_org;
            Value *negVal = AfterCI.CreateFNeg(x, "sqrt.negval");
            Value *fma = AfterCI.CreateIntrinsic(
                Intrinsic::fma,
                {rt.DoubleTy},
                {negVal, x, arg0},
                nullptr,
                "sqrt.fma"
            );
            Value *num = AfterCI.CreateFAdd(arg0_err, fma, "sqrt.num");
            Value *two = ConstantFP::get(x->getType(), 2.0);
            Value *den = AfterCI.CreateFMul(two, x, "sqrt.den");
            Value *dx = AfterCI.CreateFDiv(num, den, "sqrt.err");
            ErrorMap[CI] = dx;
            // Value *fmt = AfterCI.CreateGlobalStringPtr("sqrt_CI: x=%f, dx=%e\n");
            // AfterCI.CreateCall(rt.Printf, {fmt, x, dx});
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }

        if (N == "sin" || N == "sinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropSinDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "sin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "sin.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "sinf") {
        //     // Value *arg0 = CI->getArgOperand(0);
        //     // Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropSinFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "sinf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "sinf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "cos" || N == "cosf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropCosDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "cos.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "cos.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "cosf") {
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropCosFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "cosf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "cosf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "tan" || N == "tanf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropTanDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "tan.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "tan.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "tanf") {
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropTanFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "tanf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "tanf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "asin" || N == "asinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAsinDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "asin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "asin.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "asinf") {
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropAsinFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "asinf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "asinf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "acos" || N == "acosf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAcosDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "acos.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "acos.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "acosf") {
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropAcosFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "acosf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "acosf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "atan" || N == "atanf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAtanDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "atan.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "atan.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
            llvm::errs() << "atan\n";
        }
        // else if (N == "atanf") {
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropAtanFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "atanf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "atanf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "log" || N == "logf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropLogDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "log.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "log.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "logf") {
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropLogFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "logf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "logf.err");

        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "exp" || N == "expf") {
            // Value *arg0 = CI->getArgOperand(0);
            // Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "exp.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "exp.err");
            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
            llvm::errs() << "CI::exp\n";
        }
        // else if (N == "expf") {
        //     // Value *arg0 = CI->getArgOperand(0);
        //     // Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpFError, {arg0, arg0_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "expf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "expf.err");
        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else if (N == "pow" || N == "powf") {
            Value *arg1_org = CI->getArgOperand(1);
            Value *arg1 = isFloat ? AfterCI.CreateFPExt(arg1_org, rt.DoubleTy, "CI.arg1")   : arg1_org;
            Value *arg1_err = getError(arg1_org, rt.ZeroD, ErrorMap);
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropPowDError, {arg0, arg0_err, arg1, arg1_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "pow.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "pow.err");
            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
        }
        // else if (N == "powf") {
        //     Value *arg1 = CI->getArgOperand(1);
        //     Value *arg1_err = getError(arg1, rt.ZeroF, rt.ZeroD, ErrorMap);
        //     Value *ret = AfterCI.CreateCall(rt_mpfr.PropPowFError, {arg0, arg0_err, arg1, arg1_err});
        //     Value *x = AfterCI.CreateExtractValue(ret, {0}, "powf.val");
        //     Value *dx = AfterCI.CreateExtractValue(ret, {1}, "powf.err");
        //     ErrorMap[CI] = dx;
        //     if (EnableDebugChecks) {
        //         insertCheckError(AfterCI, x, dx, CI, rt);
        //     }
        // }
        else {
            return false;
        }
    }
    return false;
}

bool handleBinary(BinaryOperator *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterBO(BO->getNextNode());
    bool isFloat = BO->getType()->isFloatTy();
    Value *opr0_org = BO->getOperand(0);
    Value *opr1_org = BO->getOperand(1);
    Value *x_org = BO;

    Value *opr0 = isFloat ? AfterBO.CreateFPExt(opr0_org, rt.DoubleTy, "BO.opr0")   : opr0_org;
    Value *opr1 = isFloat ? AfterBO.CreateFPExt(opr1_org, rt.DoubleTy, "BO.opr1")   : opr1_org;
    Value *x = isFloat ? AfterBO.CreateFPExt(x_org, rt.DoubleTy, "BO.x")            : x_org;

    Value *opr0_err = getError(opr0_org, rt.ZeroD, ErrorMap);
    Value *opr1_err = getError(opr1_org, rt.ZeroD, ErrorMap);
    switch (BO->getOpcode()) {
        case Instruction::FAdd: {
            // Value *x = BO;
            Value *bp = AfterBO.CreateFSub(x, opr0, "fadd.bp");
            Value *ap = AfterBO.CreateFSub(x, bp, "fadd.ap");
            Value *da = AfterBO.CreateFSub(opr0, ap, "fadd.da");
            Value *db = AfterBO.CreateFSub(opr1, bp, "fadd.db");
            Value *dab = AfterBO.CreateFAdd(db, da, "fadd.dab");
            Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fadd.tmp");
            Value *dx = AfterBO.CreateFAdd(opr1_err, tmp, "fadd.err");
            ErrorMap[BO] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, x, dx, BO, rt);
            }
            return true;
        }
        case Instruction::FSub: {
            // Value *x = BO;
            Value *bp = AfterBO.CreateFSub(x, opr0, "fsub.bp");
            Value *ap = AfterBO.CreateFSub(x, bp, "fsub.ap");
            Value *da = AfterBO.CreateFSub(opr0, ap, "fsub.da");
            Value *db = AfterBO.CreateFAdd(opr1, bp, "fsub.db");
            Value *dab = AfterBO.CreateFSub(da, db, "fsub.dab");
            Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fsub.tmp");
            Value *dx = AfterBO.CreateFSub(tmp, opr1_err, "fsub.err");
            ErrorMap[BO] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, x, dx, BO, rt);
            }
            return true;
        }
        case Instruction::FMul: {
            // Value *x = BO;
            Value *negVal = AfterBO.CreateFNeg(x, "fmul.negval");
            Value *fma = AfterBO.CreateIntrinsic(
                Intrinsic::fma,
                {rt.DoubleTy},
                {opr0, opr1, negVal},
                nullptr,
                "mul.fma"
            );
            Value *adb = AfterBO.CreateFMul(opr0, opr1_err, "fmul.adb");
            Value *tmp = AfterBO.CreateFAdd(fma, adb, "fmul.tmp");
            Value *bda = AfterBO.CreateFMul(opr1, opr0_err, "fmul.bda");
            Value *dx = AfterBO.CreateFAdd(tmp, bda, "fmul.err");
            ErrorMap[BO] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, x, dx, BO, rt);
            }
            return true;
        }
        case Instruction::FDiv: {
            // Value *x = BO;
            Value *invopr0 = AfterBO.CreateFNeg(opr0, "fdiv.invopr0");
            Value *fma = AfterBO.CreateIntrinsic(
                Intrinsic::fma,
                {rt.DoubleTy},
                {x, opr1, invopr0},
                nullptr,
                "fdiv.fma"
            );
            Value *da = AfterBO.CreateFSub(opr0_err, fma);
            Value *invval = AfterBO.CreateFNeg(x, "fdiv.invval");
            Value *numer = AfterBO.CreateIntrinsic(
                Intrinsic::fma,
                {rt.DoubleTy},
                {invval, opr1_err, da},
                nullptr,
                "fdiv.numer"
            );
            Value *denom = AfterBO.CreateFAdd(opr1, opr1_err, "fdiv.denom");
            Value *dx = AfterBO.CreateFDiv(numer, denom, "fdiv.err");
            ErrorMap[BO] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, x, dx, BO, rt);
            }
            return true;
        }
        default:
            return false;
    }
}

bool handleFCmp(FCmpInst *FC, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap) {
    Value *opr0_org = FC->getOperand(0);
    if (opr0_org->getType()->isDoubleTy() || opr0_org->getType()->isFloatTy()) {
        return false;
    }
    bool isFloat = opr0_org->getType()->isFloatTy();
    IRBuilder<> AfterFC(FC->getNextNode());
    Value *opr1_org = FC->getOperand(1);

    Value *opr0 = isFloat ? AfterFC.CreateFPExt(opr0_org, rt.DoubleTy, "FC.opr0") : opr0_org;
    Value *opr1 = isFloat ? AfterFC.CreateFPExt(opr1_org, rt.DoubleTy, "FC.opr1") : opr1_org;

    Value *opr0_err = getError(opr0_org, rt.ZeroD, ErrorMap);
    Value *opr1_err = getError(opr1_org, rt.ZeroD, ErrorMap);

    Value *pred = ConstantInt::get(rt.I32Ty, (int)FC->getPredicate());
    // uint32_t id = getSiteId(FC);
    // Value *SiteId = ConstantInt::get(rt.I32Ty, id);

    AfterFC.CreateCall(rt.CheckBranch, {opr0, opr0_err, opr1, opr1_err, pred});
    // AfterFC.CreateCall(rt.CheckBranch, {opr0, opr0_err, opr1, opr1_err, pred, SiteId});
    return true;
}

bool handleFPToSI(FPToSIInst *CI, utils::RuntimeFns &rt,
                      DenseMap<const Value*, Value*> &ErrorMap) {
    Value *src_org = CI->getOperand(0);
    if (!src_org->getType()->isDoubleTy() && !src_org->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterCI(CI->getNextNode());
    bool isFloat = src_org->getType()->isFloatTy();
    Value *src = isFloat ? AfterCI.CreateFPExt(src_org, rt.DoubleTy, "conv.src") : src_org;
    Value *src_err = getError(src_org, rt.ZeroD, ErrorMap);

    Value *sval = CI;
    if (CI->getType() != rt.I32Ty) {
        sval = AfterCI.CreateSExtOrTrunc(CI, rt.I32Ty, "conv.sval");
    }
    AfterCI.CreateCall(rt.CheckConvSI, {sval, src, src_err});
    return true;
}

bool handleFPToUI(FPToUIInst *CI, utils::RuntimeFns &rt,
                      DenseMap<const Value*, Value*> &ErrorMap) {
    Value *src_org = CI->getOperand(0);
    if (!src_org->getType()->isDoubleTy() && !src_org->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterCI(CI->getNextNode());
    bool isFloat = src_org->getType()->isFloatTy();
    Value *src = isFloat ? AfterCI.CreateFPExt(src_org, rt.DoubleTy, "conv.src") : src_org;
    Value *src_err = getError(src_org, rt.ZeroD, ErrorMap);

    Value *uval = CI;
    if (CI->getType() != rt.I64Ty) {
        uval = AfterCI.CreateSExtOrTrunc(CI, rt.I64Ty, "conv.uval");
    }
    AfterCI.CreateCall(rt.CheckConvUI, {uval, src, src_err});
    return true;
}