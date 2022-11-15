//
// Created by BY210033 on 2022/11/15.
//
#include "tinylang/CodeGen/CGProcedure.h"

#include <llvm/IR/CFG.h>
#include <llvm/Support/Casting.h>

using namespace tinylang;
using namespace llvm;

void CGProcedure::writeLocalVariable(BasicBlock *BB, Decl *Decl, Value *Val) {
    assert(BB && "Basic block is nullptr");
    assert(isa<VariableDeclaration>(Decl) || isa<FormalParameterDeclaration>(Decl)
            && "Declaration must be variable or formal parameter");
    assert(Val && "Val is nullptr");
    CurrentDef[BB].Defs[Decl] = Val;
}

Value *CGProcedure::readLocalVariable(BasicBlock *BB, Decl *Decl) {
    assert(BB && "Basic block is nullptr");
    assert((isa<VariableDeclaration>(Decl) || isa<FormalParameterDeclaration>(Decl))
        && "Declaration must be variable or formal parameter");
    auto Val = CurrentDef[BB].Defs.find(Decl);
    if (Val != CurrentDef[BB].Defs.end())
        return Val->second;
    // 如果在当前块没有找到需要递归查找父级块
    return readLocalVariableRecursive(BB, Decl);
}

/**
 * 递归查找前一个基本块。以下面的while为例，条件中使用的变量有一个初始值，并且可能在循环体中更改。所以，
 * 需要收集这些定义并从中创建一条phi指令。由while语句创建的基本快包含一个循环
 * 因为我们递归搜索前一个块，必须打破这个循环，为此，需要使用一个简单的技巧--插入一条空的phi指令， 并将其记录为变量的当前值。
 * 如果在搜索中再次看到这个基本快，那么就会发现这个变量有值，我们就会使用它，搜索到此位置。在我们收集了所有值之后，必须对phi指令进行更新
 * 还有一个问题，查找时，并不是所有基本快的前块都是已知的。
 * @param BB
 * @param Decl
 * @return
 */
Value *
CGProcedure::readLocalVariableRecursive(BasicBlock *BB, Decl *Decl) {
    Value *Val = nullptr;
    if (!CurrentDef[BB].Sealed) {
        // Add incomplete phi for variable.
        PHINode *Phi = addEmptyPhi(BB, Decl);
        CurrentDef[BB].IncompletePhis[Phi] = Decl;
        Val = Phi;
    // getSinglePredecessor()，如果该块有前块，返回一个前块，否则返回空指针
    } else if (auto *PredBB = BB->getSinglePredecessor()) {
        // Only one predecessor.
        Val = readLocalVariable(PredBB, Decl);
    } else {
        // Create empty phi instruction to break potential
        // cycles.
        PHINode *Phi = addEmptyPhi(BB, Decl);
        Val = Phi;
        writeLocalVariable(BB, Decl, Val);
        addPhiOperands(BB, Decl, Phi);
    }
    writeLocalVariable(BB, Decl, Val);
    return Val;
}

PHINode *CGProcedure::addEmptyPhi(BasicBlock *BB, Decl *Decl) {
    return BB->empty() ? PHINode::Create(mapType(Decl), 0, "", BB)
                       : PHINode::Create(mapType(Decl), 0, "", &BB->front());
}

// 为了将确实的操作数加到phi指令中，我们首先搜索基本块的所有前任，然后将操作数对值和基本块加到phi指令中，然后尝试优化指令
void CGProcedure::addPhiOperands(BasicBlock *BB, Decl *Decl, PHINode *Phi) {
    for (auto I = pred_begin(BB); I != pred_end(BB); ++I) {
        Phi->addIncoming(readLocalVariable(*I, Decl), *I);
    }
    optimizePhi(Phi);
}

// 上面的形式会产生不必要的phi指令，这里进行一种优化
// 虽然SSA形式对优化是有利的，但算法通常无法理解phi指令，所以会阻碍优化，因此，我们生成的phi指令越少越好
// 如果指令只有一个操作数或操作数都是相同的值，则用这个值替换指令。如果指令没有操作数，则用特殊值Undef替换
void CGProcedure::optimizePhi(PHINode *Phi) {
    Value *Same = nullptr;
    for (Value *V : Phi->incoming_values()) {
        if (V == Same || V == Phi)
            continue;
        if (Same && V != Same)
            return;
        Same = V;
    }
    if (Same == nullptr)
        Same = UndefValue::get(Phi->getType());
    // Collect phi instructions using the one.
    // 删除了一个phi指令可能会导致其他phi指令有优化的机会。我们搜索在其他phi指令中的用法，尝试优化这些指令：
    SmallVector<PHINode *, 8> CandidatePhis;
    // Use表示值定义和它的用户之间的界限。这是一个二维链表。它支持遍历特定值定义的所有用法
    for (Use &U : Phi->uses()) {
        if (auto *P = dyn_cast<PHINode>(U.getUser()))
            if (P != Phi)
                CandidatePhis.push_back(P);
    }
    // 将Phi节点替换成对应的Value
    Phi->replaceAllUsesWith(Same);
    // 此方法将"This"从包含它的基本块中断开链接并删除它
    Phi->eraseFromParent();
    for (auto *P : CandidatePhis) {
        optimizePhi(P);
    }
    // 如果需要，算法还可以进一步改进。可以选择记住两个不同的值，而不是迭代每个phi指令的列表。
    // 在优化函数李，可以检查这两个值是否仍然在phi指令列表中，如果在就不需要优化了
    // 但目前的优化即使没有上述的操作，运行速度已经很快了
}

void CGProcedure::sealBlock(BasicBlock *BB) {
    assert(!CurrentDef[BB].Sealed &&
            "Attempt to seal already sealed block");
    for (auto PhiDecl : CurrentDef[BB].IncompletePhis) {
        addPhiOperands(BB, PhiDecl.second, PhiDecl.first);
    }
    CurrentDef[BB].IncompletePhis.clear();
    CurrentDef[BB].Sealed = true;
}

void CGProcedure::writeVariable(BasicBlock *BB, Decl *Decl, Value *Val) {
    if (auto *V = dyn_cast<VariableDeclaration>(Decl)) {
        if (V->getEnclosingDecl() == Proc)
            writeLocalVariable(BB, Decl, Val);
        else if (V->getEnclosingDecl() == CGM.getModuleDeclaration()) {
            Builder.CreateStore(Val, CGM.getGlobal(Decl));
        } else
            report_fatal_error("Nested procedures not yet supported");
    } else if (auto *FP = dyn_cast<FormalParameterDeclaration>(Decl)) {
        if (FP->isVar()) {
            Builder.CreateStore(Val, FormalParams[FP]);
        } else
            writeLocalVariable(BB, Decl, Val);
    } else
        report_fatal_error("Unsupported declaration");
}

// 如何访问全部的变量：
// 1 - 局部变量，访问readLocalVariable() 和 writeLocalVariable()
// 2 - 对于封闭过程中的局部变量，需要指向封闭框架的指针
// 3 - 对于全局变量，生成加载和存储指令
// 4 - 对于形式参数，必须区分按值传递和按引用传递(tinylang中的VAR参数).按值传递视为局部变量，按引用传递视为全局变量
Value *CGProcedure::readVariable(BasicBlock *BB, Decl *Decl) {
    if (auto *V = dyn_cast<VariableDeclaration>(Decl)) {
        if (V->getEnclosingDecl() == Proc)
            return readLocalVariable(BB, Decl);
        else if (V->getEnclosingDecl() == CGM.getModuleDeclaration()) {
            return Builder.CreateLoad(mapType(Decl), CGM.getGlobal(Decl));
        } else
            report_fatal_error("Nested procedures not yet supported");
    } else if (auto *FP = dyn_cast<FormalParameterDeclaration>(Decl)) {
        if (FP->isVar()) {
            return Builder.CreateLoad(mapType(FP)->getPointerElementType(), FormalParams[FP]);
        } else
            return readLocalVariable(BB, Decl);
    } else
        report_fatal_error("Unsupported declaration");
}

Type *CGProcedure::mapType(Decl *Decl) {
    if (auto *FP = dyn_cast<FormalParameterDeclaration>(Decl)) {
        Type *Ty = CGM.convertType(FP->getType());
        if (FP->isVar())
            Ty = Ty->getPointerTo();
        return Ty;
    }
    if (auto *V = dyn_cast<VariableDeclaration>(Decl))
        return CGM.convertType(V->getType());
    return CGM.convertType(cast<TypeDeclaration>(Decl));
}

FunctionType *
CGProcedure::createFunctionType(ProcedureDeclaration *Proc) {
    Type *ResultTy = CGM.VoidTy;
    if (Proc->getRetType()) {
        ResultTy = mapType(Proc->getRetType());
    }
    auto FormalParams = Proc->getFormalParams();
    SmallVector<Type *, 8> ParamTypes;
    for (auto FP : FormalParams) {
        Type *Ty = mapType(FP);
        ParamTypes.push_back(Ty);
    }
    return FunctionType::get(ResultTy, ParamTypes, false);
}

Function *
CGProcedure::createFunction(ProcedureDeclaration *Proc, FunctionType *FTy) {
    Function *Fn = Function::Create(Fty, GlobalValue::ExternalLinkage, CGM.mangleName(Proc), CGM.getModule());
    size_t Idx = 0;
    for (auto I = Fn->arg_begin(); I != Fn->arg_end(); ++I, ++Idx) {
        Argument *Arg = I;
        FormalParameterDeclaration *FP = Proc->getFormalParams()[Idx];
        if (FP->isVar()) {
            AttrBuilder Attr;
            TypeSize Sz = CGM.getModule()->getDataLayout().getTypeStoreSize(CGM.convertType(FP->getType()));
            Attr.addDereferenceableAttr(Sz);
            Attr.addAttribute(Attribute::NoCapture);
            Arg->addAttrs(Attr);
        }
        Arg->setName(FP->getName());
    }
    return Fn;
}

Value *CGProcedure::emitExpr(Expr *E) {
    if (auto *Infix = dyn_cast<InfixExpression>(E)) {

    }
}

void CGProcedure::emitStmt(WhileStatement *Stmt) {
    // The basic block for the condition.
    BasicBlock *WhileCondBB = BasicBlock::Create(CGM.getLLVMCtx(), "while.cond", Fn);
    BasicBlock *WhileBodyBB = BasicBlock::Create(CGM.getLLVMCtx(), "while.body", Fn);
    BasicBlock *AfterWhileBB = BasicBlock::Create(CGM.getLLVMCtx(), "after.while", Fn);

    Builder.CreateBr(WhileCondBB);
    sealBlock(Curr);
    setCurr(WhileCondBB);
    Value *Cond = emitExpr(Stmt->getCond());
    Builder.CreateCondBr(Cond, WhileCondBB, AfterWhileBB);

    setCurr(WhileBodyBB);
    emit(Stmt->getWhileStmts());
    Builder.CreateBr(WhileCondBB);
    sealBlock(Curr);
    sealBlock(WhileCondBB);

    setCurr(AfterWhileBB);
}

void CGProcedure::run(ProcedureDeclaration *Proc) {
    this->Proc = Proc;
    Fty = createFunctionType(Proc);
    Fn = createFunction(Proc, Fty);

    BasicBlock *BB = BasicBlock::Create(CGM.getLLVMCtx(), "entry", Fn);
    setCurr(BB);
    size_t Idx = 0;
    auto &Defs = CurrentDef[BB];
    for (auto I = Fn->arg_begin(); I != Fn->arg_end(); ++I, ++Idx) {
        Argument *Arg = I;
        FormalParameterDeclaration *FP = Proc->getFormalParams()[Idx];
        // Create mapping FormalParameter -> llvm::Argument for VAR parameters;
        FormalParams[FP] = Arg;
        Defs.Defs.insert(std::pair<Decl *, llvm::Value*>(FP, Arg));
    }

    for (auto *D : Proc->getDecls()) {
        if (auto *Var = dyn_cast<VariableDeclaration>(D)) {
            Type *Ty = mapType(Var);
            if (Ty->isAggregateType()) {
                Value *Val = Builder.CreateAlloca(Ty);
                Defs.Defs.insert(std::pair<Decl *, Value *>(Var, Val));
            }
        }
    }

    emit(Proc->getStmts());
    if (!Curr->getTerminator()) {
        Builder.CreateRetVoid();
    }
    sealBlock(Curr);
}

void CGProcedure::run() {}

