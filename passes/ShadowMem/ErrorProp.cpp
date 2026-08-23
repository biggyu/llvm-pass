#include "ErrorProp.h"
#include "DebugCheck.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/SmallVector.h"
// #include "llvm/IR/Intrinsics.h"
#include "fp_ops.h"
using namespace llvm;

bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &DSLMap,
                std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {

    if (!II->getType()->isDoubleTy() && !II->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterII(II->getNextNode());
    // bool isFloat = II->getType()->isFloatTy();
    Value *arg0 = nullptr, *arg0_err = nullptr;
    DSLValues arg0_dsl;
    if (II->arg_size() > 0) {
        Value *arg0_org = II->getArgOperand(0);
        if (arg0_org->getType()->isFloatTy()) {
            arg0 = AfterII.CreateFPExt(arg0_org, rt.DoubleTy, "II.arg0");
        }
        else {
            arg0 = arg0_org;
        }
        arg0_dsl = getDSL(AfterII, arg0_org, rt, DSLMap);
        arg0_err = arg0_dsl.rhat;
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
        Value *xPluse = AfterII.CreateFAdd(arg0, arg0_err, "sqrt.xpe");
        Value *sqrtxPluse = AfterII.CreateIntrinsic(Intrinsic::sqrt, {rt.DoubleTy}, {xPluse}, nullptr, "sqrt.xpe_root");
        Value *den = AfterII.CreateFAdd(x, sqrtxPluse, "sqrt.den");
        Value *dx = AfterII.CreateFDiv(num, den, "sqrt.err");
        
        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Sqrt, rt, SiteDescs);
        }
        // Value *fmt = AfterII.CreateGlobalStringPtr("sqrt_II: x=%f, dx=%e\n");
        // AfterII.CreateCall(rt.Printf, {fmt, x, dx});
        // llvm::errs() << "II::sqrt\n";
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::sin) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropSinError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "sin.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "sin.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Sin, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::cos) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropCosError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "cos.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "cos.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Cos, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::tan) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropTanError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "tan.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "tan.err");
        
        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Tan, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::asin) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropAsinError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "asin.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "asin.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Asin, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::acos) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropAcosError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "acos.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "acos.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Acos, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::atan) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropAtanError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "atan.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "atan.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Atan, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::exp) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropExpError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "exp.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "exp.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Exp, rt, SiteDescs);
        }
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
        DSLValues arg1_dsl = getDSL(AfterII, arg1_org, rt, DSLMap);
        Value *arg1_err = arg1_dsl.rhat;
        
        Value *ret = AfterII.CreateCall(rt_mpfr.PropPowError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "pow.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "pow.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg1_dsl, x_dsl, II, FpOp::Pow, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::log) {
        Value *ret = AfterII.CreateCall(rt_mpfr.PropLogError, {arg0, arg0_err});
        Value *x = AfterII.CreateExtractValue(ret, {0}, "log.val");
        Value *dx = AfterII.CreateExtractValue(ret, {1}, "log.err");

        DSLValues x_dsl = makeDSL(AfterII, x, dx, rt, rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Log, rt, SiteDescs);
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::fabs) {
        Value *x_true = AfterII.CreateFAdd(arg0, arg0_err, "fabs.x_true");
        IntegerType *IntTy = AfterII.getIntNTy(arg0->getType()->getPrimitiveSizeInBits());
        Value *sign_mask = ConstantInt::get(IntTy, APInt::getSignMask(IntTy->getBitWidth()));
        Value *x_sign = AfterII.CreateAnd(AfterII.CreateBitCast(arg0, IntTy), sign_mask);
        Value *x_true_sign = AfterII.CreateAnd(AfterII.CreateBitCast(x_true, IntTy), sign_mask);
        Value *is_same_sign = AfterII.CreateICmpEQ(x_sign, x_true_sign);

        Value *copy_x_sign = AfterII.CreateIntrinsic(Intrinsic::copysign, {arg0->getType()}, 
                                                    {ConstantFP::get(arg0->getType(), 1.0), arg0});
        Value *formula_same = AfterII.CreateFMul(copy_x_sign, arg0_err, "fabs.samesign");

        Value *absX = AfterII.CreateIntrinsic(Intrinsic::fabs, {arg0->getType()}, {arg0}, nullptr, "fabs.val");
        Value *absX_true = AfterII.CreateIntrinsic(Intrinsic::fabs, {arg0->getType()}, {x_true});

        // FSub EFT
        Value *diff = AfterII.CreateFSub(absX_true, absX, "fabs.diffsign");
        Value *bp = AfterII.CreateFSub(diff, absX_true, "fabs.bp");
        Value *ap = AfterII.CreateFSub(diff, bp, "fabs.ap");
        Value *da = AfterII.CreateFSub(absX_true, ap, "fabs.da");
        Value *db = AfterII.CreateFAdd(absX, bp, "fabs.db");
        Value *residual = AfterII.CreateFSub(da, db, "fabs.resid");
        Value *formula_diff = AfterII.CreateFSub(diff, residual, "fabs.diffsign");

        Value *dx = AfterII.CreateSelect(is_same_sign, formula_same, formula_diff, "fabs.err");

        DSLValues x_dsl = makeDSL(AfterII, absX, dx, rt, ab rt.FalseVal);
        DSLMap[II] = x_dsl;
        if (EnableDebugChecks) {
            insertCheckError(AfterII, arg0_dsl, arg0_dsl, x_dsl, II, FpOp::Unknown, rt, SiteDescs);
        }
        return true;
    }
    else {
        return false;
    }
}

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &DSLMap,
                std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {

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
    DSLValues arg0_dsl;
    if (CI->arg_size() > 0) {
        Value *arg0_org = CI->getArgOperand(0);
        if (arg0_org->getType()->isFloatTy()) {
            arg0 = AfterCI.CreateFPExt(arg0_org, rt.DoubleTy, "CI.arg0");
        }
        else {
            arg0 = arg0_org;
        }
        // arg0 = isFloat ? AfterCI.CreateFPExt(arg0_org, rt.DoubleTy, "CI.arg0")   : arg0_org;
        arg0_dsl = getDSL(AfterCI, arg0_org, rt, DSLMap);
        arg0_err = arg0_dsl.rhat;
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
            Value *xPluse = AfterCI.CreateFAdd(arg0, arg0_err, "sqrt.xpe");
            Value *sqrtxPluse = AfterCI.CreateIntrinsic(Intrinsic::sqrt, {rt.DoubleTy}, {xPluse}, nullptr, "sqrt.xpe_root");
            Value *den = AfterCI.CreateFAdd(x, sqrtxPluse, "sqrt.den");
            Value *dx = AfterCI.CreateFDiv(num, den, "sqrt.err");
            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            // Value *fmt = AfterCI.CreateGlobalStringPtr("sqrt_CI: x=%f, dx=%e\n");
            // AfterCI.CreateCall(rt.Printf, {fm,S x, dx});
            // AfterCI.CreateCall(rt.Printf,x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Sqrt, rt, SiteDescs);
            }
        }
        else if (N == "sin" || N == "sinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropSinError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "sin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "sin.err");

            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Sin, rt, SiteDescs);
            }
            return true;
        }
        else if (N == "cos" || N == "cosf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropCosError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "cos.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "cos.err");
            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Cos, rt, SiteDescs);
            }
        }
        else if (N == "tan" || N == "tanf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropTanError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "tan.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "tan.err");

            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Tan, rt, SiteDescs);
            }
        }
        else if (N == "asin" || N == "asinf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAsinError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "asin.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "asin.err");

            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Asin, rt, SiteDescs);
            }
        }
        else if (N == "acos" || N == "acosf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAcosError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "acos.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "acos.err");
            
            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Acos, rt, SiteDescs);
            }
        }
        else if (N == "atan" || N == "atanf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropAtanError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "atan.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "atan.err");
            
            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Atan, rt, SiteDescs);
            }
        }
        else if (N == "log" || N == "logf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropLogError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "log.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "log.err");
            
            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Log, rt, SiteDescs);
            }
        }
        else if (N == "exp" || N == "expf") {
            Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpError, {arg0, arg0_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "exp.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "exp.err");

            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Exp, rt, SiteDescs);
            }
        }
        else if (N == "pow" || N == "powf") {
            Value *arg1_org = CI->getArgOperand(1), *arg1 = nullptr, *arg1_err = nullptr;
            DSLValues arg1_dsl;
            if (arg1_org->getType()->isFloatTy()) {
                arg1 = AfterCI.CreateFPExt(arg1_org, rt.DoubleTy, "CI.arg1");
            }
            else {
                arg1 = arg1_org;
            }
            // Value *arg1 = isFloat ? AfterCI.CreateFPExt(arg1_org, rt.DoubleTy, "CI.arg1")   : arg1_org;
            arg1_dsl = getDSL(AfterCI, arg1_org, rt, DSLMap);
            arg1_err = arg1_dsl.rhat;

            Value *ret = AfterCI.CreateCall(rt_mpfr.PropPowError, {arg0, arg0_err, arg1, arg1_err});
            Value *x = AfterCI.CreateExtractValue(ret, {0}, "pow.val");
            Value *dx = AfterCI.CreateExtractValue(ret, {1}, "pow.err");

            DSLValues x_dsl = makeDSL(AfterCI, x, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg1_dsl, x_dsl, CI, FpOp::Pow, rt, SiteDescs);
            }
        }
        else if (N == "fabs" || N == "fabsf") {
            Value *x_true = AfterCI.CreateFAdd(arg0, arg0_err, "fabs.x_true");
            IntegerType *IntTy = AfterCI.getIntNTy(arg0->getType()->getPrimitiveSizeInBits());
            Value *sign_mask = ConstantInt::get(IntTy, APInt::getSignMask(IntTy->getBitWidth()));
            Value *x_sign = AfterCI.CreateAnd(AfterCI.CreateBitCast(arg0, IntTy), sign_mask);
            Value *x_true_sign = AfterCI.CreateAnd(AfterCI.CreateBitCast(x_true, IntTy), sign_mask);
            Value *is_same_sign = AfterCI.CreateICmpEQ(x_sign, x_true_sign);

            Value *copy_x_sign = AfterCI.CreateIntrinsic(Intrinsic::copysign, {arg0->getType()}, 
                                                        {ConstantFP::get(arg0->getType(), 1.0), arg0});
            Value *formula_same = AfterCI.CreateFMul(copy_x_sign, arg0_err, "fabs.samesign");

            Value *absX = AfterCI.CreateIntrinsic(Intrinsic::fabs, {arg0->getType()}, {arg0}, nullptr, "fabs.val");
            Value *absX_true = AfterCI.CreateIntrinsic(Intrinsic::fabs, {arg0->getType()}, {x_true});
            // FSub EFT
            Value *diff = AfterCI.CreateFSub(absX_true, absX, "fabs.diffsign");
            Value *bp = AfterCI.CreateFSub(diff, absX_true, "fsub.bp");
            Value *ap = AfterCI.CreateFSub(diff, bp, "fsub.ap");
            Value *da = AfterCI.CreateFSub(absX_true, ap, "fsub.da");
            Value *db = AfterCI.CreateFAdd(absX, bp, "fsub.db");
            Value *residual = AfterCI.CreateFSub(da, db, "fabs.resid");
            Value *formula_diff = AfterCI.CreateFSub(diff, residual, "fabs.diffsign");

            Value *dx = AfterCI.CreateSelect(is_same_sign, formula_same, formula_diff, "fabs.err");

            DSLValues x_dsl = makeDSL(AfterCI, absX, dx, rt, rt.FalseVal);
            DSLMap[CI] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterCI, arg0_dsl, arg0_dsl, x_dsl, CI, FpOp::Unknown, rt, SiteDescs);
            }
            return true;
        }
        else {
            Function *Callee = CI->getCalledFunction();
            if (Callee && !Callee->isDeclaration()) {
                IRBuilder<> BeforeCI(CI);
                SmallVector<Value *, 4> args(CI->arg_begin(), CI->arg_end());
                for (auto it = args.rbegin(); it != args.rend(); ++it) {
                    if ((*it)->getType()->isDoubleTy() || (*it)->getType()->isFloatTy()) {
                        DSLValues d = getDSL(BeforeCI, ((*it)), rt, DSLMap);
                        BeforeCI.CreateCall(rt.ShadowStackPush, {d.xhat, d.rhat, d.sign, d.ehat, d.isExact, d.relerr});
                    }
                }
                Value *outPtr = AfterCI.CreateAlloca(rt.ShadowEntryTy, nullptr, "callee.out");
                AfterCI.CreateCall(rt.ShadowStackPop, {outPtr});
                Value *ret_err = AfterCI.CreateLoad(rt.ShadowEntryTy, outPtr);
                DSLMap[CI] = extractDSL(AfterCI, ret_err);
            }
            else {
                if (CI->getType()->isDoubleTy() || CI->getType()->isFloatTy()) {
                    DSLMap[CI] = makeDSL(AfterCI, CI, rt.ZeroD, rt, rt.TrueVal);
                }
            }
            return true;
        }
    }
    return false;
}

bool handleUnary(UnaryOperator *UO, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap) {
    if (!UO->getType()->isDoubleTy() && !UO->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterUO(UO->getNextNode());
    Value *opr = UO->getOperand(0);
    DSLValues opr_dsl = getDSL(AfterUO, opr, rt, DSLMap);
    Value *opr_err = opr_dsl.rhat;
    switch (UO->getOpcode()) {
        case Instruction::FNeg : {
            Value *dx = AfterUO.CreateFNeg(opr_err, "fneg.err");
            DSLMap[UO] = makeDSL(AfterUO, UO, dx, rt, opr_dsl.isExact);
            return true;
        }
        default: 
            return false;
    }
}

bool handleBinary(BinaryOperator *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap,
                std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterBO(BO->getNextNode());
    // bool isFloat = BO->getType()->isFloatTy();
    Value *opr0_org = BO->getOperand(0), *opr0 = nullptr;
    Value *opr1_org = BO->getOperand(1), *opr1 = nullptr;
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
    DSLValues opr0_dsl = getDSL(AfterBO, opr0_org, rt, DSLMap);
    DSLValues opr1_dsl = getDSL(AfterBO, opr1_org, rt, DSLMap);
    
    Value *opr0_err = opr0_dsl.rhat;
    Value *opr1_err = opr1_dsl.rhat;
    // Value *opr0 = isFloat ? AfterBO.CreateFPExt(opr0_org, rt.DoubleTy, "BO.opr0")   : opr0_org;
    // Value *opr1 = isFloat ? AfterBO.CreateFPExt(opr1_org, rt.DoubleTy, "BO.opr1")   : opr1_org;
    // Value *x = isFloat ? AfterBO.CreateFPExt(x_org, rt.DoubleTy, "BO.x")            : x_org;

    switch (BO->getOpcode()) {
        case Instruction::FAdd: {
            Value *bp = AfterBO.CreateFSub(x, opr0, "fadd.bp");
            Value *ap = AfterBO.CreateFSub(x, bp, "fadd.ap");
            Value *da = AfterBO.CreateFSub(opr0, ap, "fadd.da");
            Value *db = AfterBO.CreateFSub(opr1, bp, "fadd.db");
            Value *dab = AfterBO.CreateFAdd(db, da, "fadd.dab");
            Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fadd.tmp");
            Value *dx = AfterBO.CreateFAdd(opr1_err, tmp, "fadd.err");

            DSLValues x_dsl = makeDSL(AfterBO, x, dx, rt, rt.FalseVal);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Add, rt, SiteDescs);
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
            
            DSLValues x_dsl = makeDSL(AfterBO, x, dx, rt, rt.FalseVal);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Sub, rt, SiteDescs);
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
            // Ignoring second-order error term
            // Value *dx = AfterBO.CreateFAdd(tmp, bda, "fmul.err");
            // Second-order
            Value *tmp2 = AfterBO.CreateFAdd(tmp, bda, "fmul.err");
            Value *dadb = AfterBO.CreateFMul(opr0_err, opr1_err, "fmul.dadb");
            Value *dx = AfterBO.CreateFAdd(tmp2, dadb, "fmul.err");
            
            DSLValues x_dsl = makeDSL(AfterBO, x, dx, rt, rt.FalseVal);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Mul, rt, SiteDescs);
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
            
            DSLValues x_dsl = makeDSL(AfterBO, x, dx, rt, rt.FalseVal);
            DSLMap[BO] = x_dsl;
            if (EnableDebugChecks) {
                insertCheckError(AfterBO, opr0_dsl, opr1_dsl, x_dsl, BO, FpOp::Div, rt, SiteDescs);
            }
            return true;
        }
        default:
            return false;
    }
}

bool handleFCmp(FCmpInst *FC, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap,
                std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    Value *opr0_org = FC->getOperand(0), *opr0 = nullptr;
    Value *opr1_org = FC->getOperand(1), *opr1 = nullptr;
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
    DSLValues opr0_dsl = getDSL(AfterFC, opr0_org, rt, DSLMap);
    DSLValues opr1_dsl = getDSL(AfterFC, opr1_org, rt, DSLMap);

    Value *pred = ConstantInt::get(rt.I64Ty, (int)FC->getPredicate());
    // uint32_t id = getSiteId(FC);
    // Value *SiteId = ConstantInt::get(rt.I32Ty, id);

    //TODO: Check
    if (EnableDebugChecks) {
        insertCheckBranch(AfterFC, opr0_dsl, opr1_dsl, pred, FC, FpOp::Branch, rt, SiteDescs);
    }
    // AfterFC.CreateCall(rt.CheckBranch, {opr0, opr0_err, opr1, opr1_err, pred, FC});
    // AfterFC.CreateCall(rt.CheckBranch, {opr0, opr0_err, opr1, opr1_err, pred, SiteId});
    return true;
}

bool handleFPToSI(FPToSIInst *CI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap,
                std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    Value *src_org = CI->getOperand(0), *src = nullptr;
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
    DSLValues src_dsl = getDSL(AfterCI, src_org, rt, DSLMap);

    Value *sval = CI;
    if (CI->getType() != rt.I32Ty) {
        sval = AfterCI.CreateSExtOrTrunc(CI, rt.I32Ty, "conv.sval");
    }
    if (EnableDebugChecks) {
        insertCheckConv(AfterCI, src_dsl, sval, CI, FpOp::ConvSI, rt, SiteDescs);
    }
    // AfterCI.CreateCall(rt.CheckConvSI, {sval, src, src_err});
    return true;
}

bool handleFPToUI(FPToUIInst *CI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap,
                std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    Value *src_org = CI->getOperand(0), *src = nullptr;

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
    DSLValues src_dsl = getDSL(AfterCI, src_org, rt, DSLMap);
    // Value *src_err = getDSL(AfterCI, src_org, rt, DSLMap).rhat;

    Value *uval = CI;
    if (CI->getType() != rt.I64Ty) {
        uval = AfterCI.CreateZExtOrTrunc(CI, rt.I64Ty, "conv.uval");
    }
    if (EnableDebugChecks) {
        insertCheckConv(AfterCI, src_dsl, uval, CI, FpOp::ConvUI, rt, SiteDescs);
    }
    // AfterCI.CreateCall(rt.CheckConvUI, {uval, src, src_err});
    return true;
}

bool handleSIToFP(SIToFPInst *SI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap) {
    if (!SI->getType()->isDoubleTy() && !SI->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterSI(SI->getNextNode());
    DSLMap[SI] = makeDSL(AfterSI, SI, rt.ZeroD, rt, rt.TrueVal);
    return true;
}
                
bool handleUIToFP(UIToFPInst *UI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap) {
    if (!UI->getType()->isDoubleTy() && !UI->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterUI(UI->getNextNode());
    DSLMap[UI] = makeDSL(AfterUI, UI, rt.ZeroD, rt, rt.TrueVal);
    return true;
}