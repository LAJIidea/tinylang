//
// Created by BY210033 on 2022/11/8.
//

#ifndef TINYLANG_SEMA_H
#define TINYLANG_SEMA_H

#include "tinylang/AST/AST.h"
#include "tinylang/Basic/Diagnostic.h"
#include "tinylang/Sema/Scope.h"

#include <memory>

namespace tinylang {

    class Sema {
        friend class EnterDeclScope;
        void enterScope(Decl *D);
        void leaveScope();

        bool isOperatorForType(tok::TokenKind Op,
                               TypeDeclaration *Ty);

        void checkFormalAndActualParameters(
                llvm::SMLoc Loc, const FormalParamList &Formals,
                const ExprList &Actuals);

        Scope *CurrentScope;
        Decl *CurrentDecl;
        DiagnosticsEngine &Diags;

        TypeDeclaration *IntegerType;
        TypeDeclaration *BooleanType;
        BooleanLiteral *TrueLiteral;
        BooleanLiteral *FalseLiteral;
        ConstantDeclaration *TrueConst;
        ConstantDeclaration *FalseConst;

    public:
        Sema(DiagnosticsEngine &Diags)
            : CurrentScope(nullptr), CurrentDecl(nullptr), Diags(Diags)
        {
            initialize();
        }

        void initialize();

        ModuleDeclaration *actOnModuleDeclaration(llvm::SMLoc Loc, llvm::StringRef Name);

        void actOnModuleDeclaration(ModuleDeclaration *ModDecl,
                                    llvm::SMLoc Loc, llvm::StringRef Name,
                                    DeclList &Decls, StmtList &Stmts);

        void actOnImport(llvm::StringRef ModuleName, IdentList &Ids);
        void actOnConstantDeclaration(DeclList &Decls, llvm::SMLoc Loc,
                                      llvm::StringRef Name, Expr *E);
        void actOnAliasTypeDeclaration(DeclList &Decls, llvm::SMLoc Loc,
                                       llvm::StringRef Name, Decl *D);
        void actOnArrayTypeDeclaration(DeclList &Decls, llvm::SMLoc Loc,
                                       llvm::StringRef Name, Expr *E, Decl *D);
        void actOnPointerTypeDeclaration(DeclList &Decls, llvm::SMLoc Loc,
                                         llvm::StringRef Name, Decl *D);
        void actOnFieldDeclaration(FieldList &Fields, IdentList &Ids, Decl *D);
        void actOnRecordTypeDeclaration(DeclList &Decls, llvm::SMLoc Loc,
                                        llvm::StringRef Name, const FieldList &Fields);
        void actOnVariableDeclaration(DeclList &Decls, IdentList &Ids, Decl *D);

        void actOnFormalParameterDeclaration(FormalParamList &Params, IdentList &Ids,
                                             Decl *D, bool IsVar);

        ProcedureDeclaration *
        actOnProcedureDeclaration(llvm::SMLoc Loc, llvm::StringRef Name);
        void actOnProcedureHeading(ProcedureDeclaration *ProcDecl,
                                   FormalParamList &Params, Decl *RetType);
        void actOnProcedureDeclaration(
                ProcedureDeclaration *ProcDecl, llvm::SMLoc Loc,
                llvm::StringRef Name, DeclList &Decls, StmtList &Stmts);

        void actOnAssignment(StmtList &Stmts, llvm::SMLoc Loc, Expr *D, Expr *E);
        void actOnProcCall(StmtList &Stmts, llvm::SMLoc Loc, Decl *D, ExprList &Params);
        void actOnIfStatement(StmtList &Stmts, llvm::SMLoc Loc,
                              Expr *Cond, StmtList &IfStmts,
                              StmtList &ElseStmts);
        void actOnWhileStatement(StmtList &Stmts, llvm::SMLoc Loc,
                                 Expr *Cond, StmtList &WhileStmts);
        void actOnReturnStatement(StmtList &Stmts, llvm::SMLoc Loc, Expr *RetVal);

        Expr *actOnExpression(Expr *Left, Expr *Right,
                              const OperatorInfo &Op);
        Expr *actOnSimpleExpression(Expr *Left, Expr *Right,
                                    const OperatorInfo &Op);
        Expr *actOnTerm(Expr *Left, Expr *Right, const OperatorInfo &Op);

        Expr *actOnPrefixExpression(Expr *E, const OperatorInfo &Op);
        Expr *actOnIntegerLiteral(llvm::SMLoc Loc, llvm::StringRef Literal);
        void actOnIndexSelector(Expr *Desig, llvm::SMLoc Loc, Expr *E);
        void actOnFieldSelector(Expr *Desig, llvm::SMLoc Loc, llvm::StringRef Name);
        void actOnDereferenceSelector(Expr *Desig, llvm::SMLoc Loc);
        Expr *actOnDesignator(Decl *D);
        Expr *actOnVariable(Decl *D);
        Expr *actOnFunctionCall(Decl *D, ExprList &Params);
        Decl *actOnQualIdentPart(Decl *Prev, llvm::SMLoc Loc, llvm::StringRef Name);
    };

    class EnterDeclScope {
        Sema &Semantics;

    public:
        EnterDeclScope(Sema &Sematics, Decl *D)
            : Semantics(Sematics) {
            Sematics.enterScope(D);
        }
        ~EnterDeclScope() { Semantics.leaveScope(); }
    };

} // namespace tinylang

#endif //TINYLANG_SEMA_H
