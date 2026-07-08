#include "ErrorProp.h"
#include "DebugCheck.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/SmallVector.h"
// #include "llvm/IR/Intrinsics.h"
using namespace llvm;

static Value* getError(Value *v, Constant *ZeroD, 
                        DenseMap<const Value*, Value*> &ErrorMap) {
    auto it = ErrorMap.find(v);
    if (it != ErrorMap.end()) {
        return it->second;
    }
    // if (!isa<ConstantFP>(v)) {
    //     llvm::errs() << "ErrorProp getError MISS on: ";
    //     v->print(llvm::errs());
    //     llvm::errs() << "\n";
    // }
    return ZeroD;
}

bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap) {

    if (!II->getType()->isDoubleTy() && !II->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterII(II->getNextNode());
    // bool isFloat = II->getType()->isFloatTy();
    Value *arg0 = nullptr, *arg0_err = nullptr;
    if (II->arg_size() > 0) {
        Value *arg0_org = II->getArgOperand(0);
        if (arg0_org->getType()->isFloatTy()) {
            arg0 = AfterII.CreateFPExt(arg0_org, rt.DoubleTy, "II.arg0");
        }
        else {
            arg0 = arg0_org;
        }
        if (arg0) {
            arg0_err = getError(arg0_org, rt.ZeroD, ErrorMap);
        }
    
    }
    //TODO: Exception Handling(arg0 < 0)
    if (II->getIntrinsicID() == Intrinsic::sqrt) {
        Value *x_org = II, *x = nullptr;
        if (x_org->getType()->isFloatTy()) {
            x = AfterII.CreateFPExt(x_org, rt.DoubleTy, "II.x");
        }
        else {
            x = x_org;
        }
        // Value *x = isFloat ? AfterII.CreateFPExt(x_org, rt.DoubleTy, "II.x")        : x_org;
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
        // llvm::errs() << "II::sqrt\n";
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
        // llvm::errs() << "II::exp\n";
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::pow) {
        Value *arg1_org = II->getArgOperand(1), *arg1 = nullptr;
        if (arg1_org->getType()->isFloatTy()) {
            arg1 = AfterII.CreateFPExt(arg1_org, rt.DoubleTy, "II.arg1");
        }
        else {
            arg1 = arg1_org;
        }
        // Value *arg1 = isFloat ? AfterII.CreateFPExt(arg1_org, rt.DoubleTy, "II.arg1")   : arg1_org;
        if (arg1) {
            Value *arg1_err = getError(arg1_org, rt.ZeroD, ErrorMap);
        }
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
        return true;
    }
    else {
        return false;
    }
}

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap) {

    if (Function *Callee = CI->getCalledFunction()) {
        if (isRuntimeFunction(*Callee)) {
            return false;
        }
    }
    if (!CI->getType()->isDoubleTy() && !CI->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterCI(CI->getNextNode());
    // bool isFloat = CI->getType()->isFloatTy();
    
    Value *arg0 = nullptr, *arg0_err = nullptr;
    if (CI->arg_size() > 0) {
        Value *arg0_org = CI->getArgOperand(0);
        if (arg0_org->getType()->isFloatTy()) {
            arg0 = AfterCI.CreateFPExt(arg0_org, rt.DoubleTy, "CI.arg0");
        }
        else {
            arg0 = arg0_org;
        }
        // arg0 = isFloat ? AfterCI.CreateFPExt(arg0_org, rt.DoubleTy, "CI.arg0")   : arg0_org;
        if (arg0) {
            arg0_err = getError(arg0_org, rt.ZeroD, ErrorMap);
        }
    }
    
    if (Function *Callee = CI->getCalledFunction()) {
        StringRef N = Callee->getName();
        //TODO: Exception Handling(arg0 < 0)
        if (N == "sqrt" || N == "sqrtf") {
            Value *x_org = CI, *x = nullptr;
            if (x_org->getType()->isFloatTy()) {
                x = AfterCI.CreateFPExt(x_org, rt.DoubleTy, "CI.x");
            }
            else {
                x = x_org;
            }
            // Value *x = isFloat ? AfterCI.CreateFPExt(x_org, rt.DoubleTy, "CI.x")    : x_org;
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
            return true;
            // llvm:errs() << "CI::sqrt\n";
        }

        if (N == "sin" || N == "sinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropSinDError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "sin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "sin.err");

            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
            return true;
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
            return true;
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
            return true;
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
            return true;
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
            return true;
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
            return true;
            // llvm::errs() << "atan\n";
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
            return true;
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
            return true;
            // llvm::errs() << "CI::exp\n";
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
            Value *arg1_org = CI->getArgOperand(1), *arg1 = nullptr, *arg1_err = nullptr;
            if (arg1_org->getType()->isFloatTy()) {
                arg1 = AfterCI.CreateFPExt(arg1_org, rt.DoubleTy, "CI.arg1");
            }
            else {
                arg1 = arg1_org;
            }
            // Value *arg1 = isFloat ? AfterCI.CreateFPExt(arg1_org, rt.DoubleTy, "CI.arg1")   : arg1_org;
            if (arg1) {
                arg1_err = getError(arg1_org, rt.ZeroD, ErrorMap);
            }
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropPowDError, {arg0, arg0_err, arg1, arg1_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "pow.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "pow.err");
            ErrorMap[CI] = dx;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, x, dx, CI, rt);
            }
            return true;
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
            Function *Callee = CI->getCalledFunction();
            if (Callee && !Callee->isDeclaration()) {
                IRBuilder<> BeforeCI(CI);
                SmallVector<Value *, 4> args(CI->arg_begin(), CI->arg_end());
                for (auto it = args.rbegin(); it != args.rend(); ++it) {
                    if ((*it)->getType()->isDoubleTy() || (*it)->getType()->isFloatTy()) {
                        BeforeCI.CreateCall(rt.ShadowStackPush, {getError((*it), rt.ZeroD, ErrorMap)});
                    }
                }
                Value *ret_err = AfterCI.CreateCall(rt.ShadowStackPop, {});
                ErrorMap[CI] = ret_err;
            }
            else {
                ErrorMap[CI] = rt.ZeroD;
            }
            return true;
        }
    }
    return false;
}

bool handleUnary(UnaryOperator *UO, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (!UO->getType()->isDoubleTy() && !UO->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterUO(UO->getNextNode());
    // Value *opr = UO->getOperand(0);
    Value *opr_err = getError(UO->getOperand(0), rt.ZeroD, ErrorMap);
    switch (UO->getOpcode()) {
        case Instruction::FNeg : {
            Value *dx = AfterUO.CreateFNeg(opr_err, "fneg.err");
            ErrorMap[UO] = dx;
            return true;
        }
        default: 
            return false;
    }
}


bool handleBinary(BinaryOperator *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterBO(BO->getNextNode());
    // bool isFloat = BO->getType()->isFloatTy();
    Value *opr0_org = BO->getOperand(0), *opr0 = nullptr, *opr0_err = nullptr;
    Value *opr1_org = BO->getOperand(1), *opr1 = nullptr, *opr1_err = nullptr;
    Value *x_org = BO, *x = nullptr;

    if (opr0_org->getType()->isFloatTy()) {
        opr0 = AfterBO.CreateFPExt(opr0_org, rt.DoubleTy, "BO.opr0");
        opr1 = AfterBO.CreateFPExt(opr1_org, rt.DoubleTy, "BO.opr1");
        x = AfterBO.CreateFPExt(x_org, rt.DoubleTy, "BO.x");
    }
    else {
        opr0 = opr0_org;
        opr1 = opr1_org;
        x = x_org;
    }
    if (opr0) {
        opr0_err = getError(opr0_org, rt.ZeroD, ErrorMap);
    }
    if (opr1) {
        opr1_err = getError(opr1_org, rt.ZeroD, ErrorMap);
    }
    // Value *opr0 = isFloat ? AfterBO.CreateFPExt(opr0_org, rt.DoubleTy, "BO.opr0")   : opr0_org;
    // Value *opr1 = isFloat ? AfterBO.CreateFPExt(opr1_org, rt.DoubleTy, "BO.opr1")   : opr1_org;
    // Value *x = isFloat ? AfterBO.CreateFPExt(x_org, rt.DoubleTy, "BO.x")            : x_org;

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
            // Second-order
            // Value *tmp2 = AfterBO.CreateFAdd(tmp, bda, "fmul.err");
            // Value *dadb = AfterBO.CreateFMul(opr0_err, opr1_err, "fmul.dadb");
            // Value *dx = AfterBO.CreateFAdd(tmp2, dadb, "fmul.err");
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
    Value *opr0_org = FC->getOperand(0), *opr0 = nullptr, *opr0_err = nullptr;
    Value *opr1_org = FC->getOperand(1), *opr1 = nullptr, *opr1_err = nullptr;
    if (!opr0_org->getType()->isDoubleTy() && !opr0_org->getType()->isFloatTy()) {
        return false;
    }
    // bool isFloat = opr0_org->getType()->isFloatTy();
    IRBuilder<> AfterFC(FC->getNextNode());

    if (opr0_org->getType()->isFloatTy()) {
        opr0 = AfterFC.CreateFPExt(opr0_org, rt.DoubleTy, "FC.opr0");
        opr1 = AfterFC.CreateFPExt(opr1_org, rt.DoubleTy, "FC.opr1");
    }
    else {
        opr0 = opr0_org;
        opr1 = opr1_org;
    }
    if (opr0) {
        opr0_err = getError(opr0_org, rt.ZeroD, ErrorMap);
    }
    if (opr1) {
        opr1_err = getError(opr1_org, rt.ZeroD, ErrorMap);
    }


    Value *pred = ConstantInt::get(rt.I64Ty, (int)FC->getPredicate());
    // uint32_t id = getSiteId(FC);
    // Value *SiteId = ConstantInt::get(rt.I32Ty, id);

    AfterFC.CreateCall(rt.CheckBranch, {opr0, opr0_err, opr1, opr1_err, pred, FC});
    // AfterFC.CreateCall(rt.CheckBranch, {opr0, opr0_err, opr1, opr1_err, pred, SiteId});
    return true;
}

bool handleFPToSI(FPToSIInst *CI, utils::RuntimeFns &rt,
                      DenseMap<const Value*, Value*> &ErrorMap) {
    Value *src_org = CI->getOperand(0), *src = nullptr, *src_err = nullptr;
    if (!src_org->getType()->isDoubleTy() && !src_org->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterCI(CI->getNextNode());
    // bool isFloat = src_org->getType()->isFloatTy();
    if (src_org->getType()->isFloatTy()) {
        src = AfterCI.CreateFPExt(src_org, rt.DoubleTy, "conv.src");
    }
    else {
        src = src_org;
    }
    if (src) {
        src_err = getError(src_org, rt.ZeroD, ErrorMap);
    }

    Value *sval = CI;
    if (CI->getType() != rt.I32Ty) {
        sval = AfterCI.CreateSExtOrTrunc(CI, rt.I32Ty, "conv.sval");
    }
    AfterCI.CreateCall(rt.CheckConvSI, {sval, src, src_err});
    return true;
}

bool handleFPToUI(FPToUIInst *CI, utils::RuntimeFns &rt,
                      DenseMap<const Value*, Value*> &ErrorMap) {
    Value *src_org = CI->getOperand(0), *src = nullptr, *src_err = nullptr;
    if (!src_org->getType()->isDoubleTy() && !src_org->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterCI(CI->getNextNode());
    // bool isFloat = src_org->getType()->isFloatTy();
    if (src_org->getType()->isFloatTy()) {
        src = AfterCI.CreateFPExt(src_org, rt.DoubleTy, "conv.src");
    }
    else {
        src = src_org;
    }
    if (src) {
        src_err = getError(src_org, rt.ZeroD, ErrorMap);
    }

    Value *uval = CI;
    if (CI->getType() != rt.I64Ty) {
        uval = AfterCI.CreateSExtOrTrunc(CI, rt.I64Ty, "conv.uval");
    }
    AfterCI.CreateCall(rt.CheckConvUI, {uval, src, src_err});
    return true;
}

bool handleSIToFP(SIToFPInst *SI, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (!SI->getType()->isDoubleTy() && !SI->getType()->isFloatTy()) {
        return false;
    }
    ErrorMap[SI] = rt.ZeroD;
    return true;
}
                
bool handleUIToFP(UIToFPInst *UI, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (!UI->getType()->isDoubleTy() && !UI->getType()->isFloatTy()) {
        return false;
    }
    ErrorMap[UI] = rt.ZeroD;
    return true;
}