//
// Created by BY210033 on 2022/11/15.
//

#ifndef TINYLANG_CGPROCEDURE_H
#define TINYLANG_CGPROCEDURE_H

#include "tinylang/CodeGen/CGModule.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

namespace llvm {
    class Function;
}

namespace tinylang {

    class CGProcedure {
        CGModule &CGM;
        llvm::IRBuilder<> Builder;

        llvm::BasicBlock *Curr;

        ProcedureDeclaration *Proc;
        llvm::FunctionType *Fty;
        llvm::Function *Fn;

        // 使用BasicBlockDef来保存单个块的信息：
        struct BasicBlockDef {
            // llvm::value表示一个SSA形式的值
            // 某些情况不能直接使用Value类，例如优化器检测到值%1和%2总是相同的，那么可以将%2替换为%1，这个会改变标签不会改变计算.
            // 这种情况需要值句柄，要跟踪替换，可以使用llvm::TrackingVH<>类
            // 变量或形参映射到它的定义，这里用于存储局部变量
            llvm::DenseMap<Decl *, llvm::TrackingVH<llvm::Value>> Defs;
            // 存取那些未完成的 phi节点指令
            llvm::DenseMap<llvm::PHINode *, Decl *> IncompletePhis;
            unsigned Sealed : 1;

            BasicBlockDef() : Sealed(0) {}
        };

        llvm::DenseMap<llvm::BasicBlock *, BasicBlockDef> CurrentDef;

        void writeLocalVariable(llvm::BasicBlock *BB, Decl *Decl, llvm::Value *Val);
        llvm::Value *readLocalVariable(llvm::BasicBlock *BB, Decl *Decl);
        llvm::Value *readLocalVariableRecursive(llvm::BasicBlock *BB, Decl *Decl);
        llvm::PHINode *addEmptyPhi(llvm::BasicBlock *BB, Decl *Decl);
        llvm::Value *addPhiOperands(llvm::BasicBlock *BB, Decl *Decl, llvm::PHINode *Phi);
        llvm::Value *optimizePhi(llvm::PHINode *Phi);
        void sealBlock(llvm::BasicBlock *BB);

        llvm::DenseMap<FormalParameterDeclaration *, llvm::Argument *> FormalParams;
        llvm::DenseMap<Decl *, llvm::DILocalVariable *> DIVariables;

        void writeVariable(llvm::BasicBlock *BB, Decl *Decl, llvm::Value *Val);
        llvm::Value *readVariable(llvm::BasicBlock *BB, Decl *Decl, bool LoadVal = true);

        llvm::Type *mapType(Decl *Decl);
        llvm::FunctionType *createFunctionType(ProcedureDeclaration *Proc);
        llvm::Function *createFunction(ProcedureDeclaration *Proc, llvm::FunctionType *FTy);
    protected:

        /**
         * llvm ir拥有很多不同的块，这里是设置当前块，并告诉Builder接下来的语句插入哪个块中
         * @param BB
         */
        void setCurr(llvm::BasicBlock *BB) {
            Curr = BB;
            Builder.SetInsertPoint(Curr);
        }

        llvm::BasicBlock *createBasicBlock(const llvm::Twine &Name, llvm::BasicBlock *InsertBefore = nullptr) {
            return llvm::BasicBlock::Create(CGM.getLLVMCtx(), Name, Fn, InsertBefore);
        }

        llvm::Value *emitInfixExpr(InfixExpression *E);
        llvm::Value *emitPrefixExpr(PrefixExpression *E);
        llvm::Value *emitExpr(Expr *E);

        void emitStmt(AssignmentStatement *Stmt);
        void emitStmt(ProcedureCallStatement *Stmt);
        void emitStmt(IfStatement *Stmt);
        void emitStmt(WhileStatement *Stmt);
        void emitStmt(ReturnStatement *Stmt);
        void emit(const StmtList &Stmts);

    public:
        CGProcedure(CGModule &CGM)
            : CGM(CGM), Builder(CGM.getLLVMCtx()), Curr(nullptr) {}

        void run(ProcedureDeclaration *Proc);
        void run();
    };

} // namespace tinylang

#endif //TINYLANG_CGPROCEDURE_H
