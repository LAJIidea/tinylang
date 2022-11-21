//
// Created by BY210033 on 2022/11/8.
//

#ifndef TINYLANG_AST_H
#define TINYLANG_AST_H

#include "tinylang/Basic/TokenKinds.h"

#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/SMLoc.h>

#include <string>
#include <vector>

namespace tinylang {

    class Decl;
    class FormalParameterDeclaration;
    class Expr;
    class Selector;
    class Stmt;
    class TypeDeclaration;

    using DeclList = std::vector<Decl *>;
    using FormalParamList = std::vector<FormalParameterDeclaration *>;
    using ExprList = std::vector<Expr *>;
    using SelectorList = std::vector<Selector *>;
    using StmtList = std::vector<Stmt *>;

    class Ident {
        llvm::SMLoc Loc;
        llvm::StringRef Name;

    public:
        Ident(llvm::SMLoc Loc, const llvm::StringRef &Name)
            : Loc(Loc), Name(Name) {}
        llvm::SMLoc getLocation() { return Loc; }
        const llvm::StringRef &getName() { return Name; }
    };

    using IdentList = std::vector<std::pair<llvm::SMLoc, llvm::StringRef>>;

    class Field {
        llvm::SMLoc Loc;
        llvm::StringRef Name;
        TypeDeclaration *Type;

    public:
        Field(llvm::SMLoc Loc, const llvm::StringRef &Name, TypeDeclaration *Type)
            : Loc(Loc), Name(Name), Type(Type) {}
        llvm::SMLoc getLoc() const { return Loc; }
        const llvm::StringRef &getName() const { return Name; }
        TypeDeclaration *getType() const { return Type; }
    };
    using FieldList = std::vector<Field>;

    class Decl {
    public:
        enum DeclKind {
            DK_Module,
            DK_Const,
            DK_AliasType,
            DK_ArrayType,
            DK_PervasiveType,
            DK_PointerType,
            DK_RecordType,
            DK_Type,
            DK_Var,
            DK_Param,
            DK_Proc
        };

    private:
        const DeclKind Kind;

    protected:
        Decl *EnClosingDecl;
        llvm::SMLoc Loc;
        llvm::StringRef Name;

    public:
        Decl(DeclKind Kind, Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name)
            : Kind(Kind), EnClosingDecl(EnclosingDecl), Loc(Loc), Name(Name) {}

        DeclKind getKind() const { return Kind; }
        llvm::SMLoc getLocation() { return Loc; }
        llvm::StringRef getName() { return Name; }
        Decl *getEnclosingDecl() { return EnClosingDecl; }
    };

    class ModuleDeclaration : public Decl {
        DeclList Decls;
        StmtList Stmts;

    public:
        ModuleDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name)
            : Decl(DK_Module, EnclosingDecl, Loc, Name) {}

        ModuleDeclaration(Decl *EnclosingDecl, llvm::SMLoc, llvm::StringRef Name,
                          DeclList &Decls, StmtList &Stmts)
            : Decl(DK_Module, EnclosingDecl, Loc, Name), Decls(Decls), Stmts(Stmts) {}

        const DeclList &getDecls() { return Decls; }
        void setDecls(DeclList &D) { Decls = D; }
        const StmtList &getStmts() { return Stmts; }
        void SetStmts(StmtList &L) { Stmts = L; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_Module;
        }
    };


    class ConstantDeclaration : public Decl {
        Expr *E;

    public:
        ConstantDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name, Expr *E)
            : Decl(DK_Const, EnclosingDecl, Loc, Name), E(E) {}

        Expr *getExpr() { return E; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_Const;
        }
    };

    class TypeDeclaration : public Decl {
    public:
        TypeDeclaration(DeclKind Kind, Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name)
            : Decl(DK_Type, EnclosingDecl, Loc, Name) {}

        static bool classof(const Decl *D) {
            return D->getKind() >= DK_AliasType &&
                   D->getKind() <= DK_RecordType;
        }
    };

    class AliasTypeDeclaration : public TypeDeclaration {
        TypeDeclaration *Type;

    public:
        AliasTypeDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name,
                             TypeDeclaration *Type)
            : TypeDeclaration(DK_AliasType, EnclosingDecl, Loc, Name), Type(Type) {}

        TypeDeclaration *getType() const { return Type; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_AliasType;
        }
    };

    class ArrayTypeDeclaration : public TypeDeclaration {
        Expr *Nums;
        TypeDeclaration *Type;

    public:
        ArrayTypeDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name, Expr *Nums,
                             TypeDeclaration *Type)
            : TypeDeclaration(DK_ArrayType, EnclosingDecl, Loc, Name), Nums(Nums), Type(Type) {}

        Expr *getNums() const { return Nums; }
        TypeDeclaration *getType() const { return Type; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_ArrayType;
        }
    };

    class PervasiveTypeDeclaration : public TypeDeclaration {
    public:
        PervasiveTypeDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name)
            : TypeDeclaration(DK_PervasiveType, EnclosingDecl, Loc, Name) {}

        static bool classof(const Decl *D) {
            return D->getKind() == DK_PervasiveType;
        }
    };

    class PointerTypeDeclaration : public TypeDeclaration {
        TypeDeclaration *Type;

    public:
        PointerTypeDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name,
                               TypeDeclaration *Type)
            : TypeDeclaration(DK_PointerType, EnclosingDecl, Loc, Name), Type(Type) {}

        TypeDeclaration *getType() const { return Type; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_PointerType;
        }
    };

    class RecordTypeDeclaration : public TypeDeclaration {
        FieldList Fields;

    public:
        RecordTypeDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name,
                              const FieldList &Fields)
            : TypeDeclaration(DK_RecordType, EnclosingDecl, Loc, Name), Fields(Fields) {}

        const FieldList &getFields() const { return Fields; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_RecordType;
        }
    };

    class VariableDeclaration : public Decl {
        Expr *E;
        TypeDeclaration *Ty;

    public:
        VariableDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name,
                            TypeDeclaration *Ty)
            : Decl(DK_Var, EnclosingDecl, Loc, Name), Ty(Ty) {}

        Expr *getExpr() { return E; }

        TypeDeclaration *getType() { return Ty; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_Var;
        }
    };

    class FormalParameterDeclaration : public Decl {
        TypeDeclaration* Ty;
        bool IsVar;

    public:
        FormalParameterDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name,
                                   TypeDeclaration *Ty, bool IsVar)
            : Decl(DK_Param, EnclosingDecl, Loc, Name), Ty(Ty), IsVar(IsVar) {}

        bool isVar() { return IsVar; }
        TypeDeclaration *getType() { return Ty; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_Param;
        }
    };

    class ProcedureDeclaration : public Decl {
        FormalParamList Params;
        TypeDeclaration *RetType;
        DeclList Decls;
        StmtList Stmts;

    public:
        ProcedureDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name)
            : Decl(DK_Proc, EnclosingDecl, Loc, Name) {}

        ProcedureDeclaration(Decl *EnclosingDecl, llvm::SMLoc Loc, llvm::StringRef Name,
                             FormalParamList &Params, TypeDeclaration *RetType,
                             DeclList &Decls, StmtList &Stmts)
            : Decl(DK_Proc, EnclosingDecl, Loc, Name), Params(Params), RetType(RetType),
              Decls(Decls), Stmts(Stmts) {}

        const FormalParamList &getFormalParams() {
            return Params;
        }

        void setFormParams(FormalParamList &FP) { Params = FP; }
        TypeDeclaration *getRetType() { return RetType; }
        void setRetType(TypeDeclaration *Ty) { RetType = Ty; }

        const DeclList &getDecls() { return Decls; }
        void setDecls(DeclList &D) { Decls = D; }
        const StmtList &getStmts() { return Stmts; }
        void setStmts(StmtList &L) { Stmts = L; }

        static bool classof(const Decl *D) {
            return D->getKind() == DK_Proc;
        }
    };

    class OperatorInfo {
        llvm::SMLoc Loc;
        uint32_t Kind : 16;
        uint32_t IsUnspecified : 1;

    public:
        OperatorInfo()
            : Loc(), Kind(tok::unknown), IsUnspecified(true) {}
        OperatorInfo(llvm::SMLoc Loc, tok::TokenKind Kind, bool IsUnspecified = false)
            : Loc(Loc), Kind(Kind), IsUnspecified(IsUnspecified) {}

        llvm::SMLoc getLocation() const { return Loc; }
        tok::TokenKind getKind() const {
            return static_cast<tok::TokenKind>(Kind);
        }
        bool isUnspecified() const { return IsUnspecified; }
    };

    class Expr {
    public:
        enum ExprKind {
            EK_Infix,
            EK_Prefix,
            EK_Int,
            EK_Bool,
            EK_Designator,
            EK_Var,
            EK_Const,
            EK_Func,
        };

    private:
        const ExprKind Kind;
        TypeDeclaration *Ty;
        bool IsConstant;

    protected:
        Expr(ExprKind Kind, TypeDeclaration *Ty, bool IsConst)
            : Kind(Kind), Ty(Ty), IsConstant(IsConst) {}

    public:
        ExprKind getKind() const { return Kind; }
        TypeDeclaration *getType() { return Ty; }
        void setType(TypeDeclaration *T) {  Ty = T; }
        bool isConst() { return IsConstant; }
    };

    class InfixExpression : public Expr {
        Expr *Left;
        Expr *Right;
        const OperatorInfo Op;

    public:
        InfixExpression(Expr *Left, Expr *Right, OperatorInfo Op, TypeDeclaration *Ty,
                        bool IsConst)
            : Expr(EK_Infix, Ty, IsConst), Left(Left), Right(Right), Op(Op) {}

        Expr *getLeft() { return Left; }
        Expr *getRight() { return Right; }
        const OperatorInfo &getOperatorInfo() { return Op; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Infix;
        }
    };

    class PrefixExpression : public Expr {
        Expr *E;
        const OperatorInfo Op;

    public:
        PrefixExpression(Expr* E, OperatorInfo Op, TypeDeclaration *Ty, bool IsConst)
            : Expr(EK_Prefix, Ty, IsConst), E(E), Op(Op) {}

        Expr *getExpr() { return E; }
        const OperatorInfo &getOperatorInfo() { return Op; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Prefix;
        }
    };

    class IntegerLiteral : public Expr {
        llvm::SMLoc Loc;
        llvm::APSInt Value;

    public:
        IntegerLiteral(llvm::SMLoc Loc, const llvm::APSInt &Value, TypeDeclaration *Ty)
            : Expr(EK_Int, Ty, true), Loc(Loc), Value(Value) {}
        llvm::APSInt &getValue() { return Value; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Int;
        }
    };

    class BooleanLiteral : public Expr {
        bool Value;

    public:
        BooleanLiteral(bool Value, TypeDeclaration *Ty)
            : Expr(EK_Bool, Ty, true), Value(Value) {}

        bool getValue() { return Value; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Bool;
        }
    };

    class Selector {
    public:
        enum SelectorKind {
            SK_Index,
            SK_Field,
            SK_Dereference
        };

    private:
        const SelectorKind Kind;

        // The type decribes the base type.
        // E.g. the component type of an index selector
        TypeDeclaration *Type;

    protected:
        Selector(SelectorKind Kind, TypeDeclaration *Type)
            : Kind(Kind), Type(Type) {}

    public:
        SelectorKind getKind() const { return Kind; }
        TypeDeclaration *getType() const { return Type; }
    };

    class IndexSelector : public Selector {
        Expr *Index;

    public:
        IndexSelector(Expr *Index, TypeDeclaration *Type)
            : Selector(SK_Index, Type), Index(Index) {}

        Expr *getIndex() const { return Index; }

        static bool classof(const Selector *Sel) {
            return Sel->getKind() == SK_Index;
        }
    };

    class FieldSelector : public Selector {
        uint32_t Index;
        llvm::StringRef Name;

    public:
        FieldSelector(uint32_t Index, llvm::StringRef Name,
                      TypeDeclaration *Type)
            : Selector(SK_Field, Type), Index(Index), Name(Name) {}

        uint32_t getIndex() const { return Index; }
        const llvm::StringRef &getname() const { return Name; }

        static bool classof(const Selector *Sel) {
            return Sel->getKind() == SK_Field;
        }
    };

    class DereferenceSelector :public Selector {
    public:
        DereferenceSelector(TypeDeclaration *Type)
            : Selector(SK_Dereference, Type) {}

        static bool classof(const Selector *Sel) {
            return Sel->getKind() == SK_Dereference;
        }
    };

    class Designator : public Expr {
        Decl *Var;
        SelectorList Selectors;

    public:
        Designator(VariableDeclaration *Var)
            : Expr(EK_Designator, Var->getType(), false), Var(Var) {}

        Designator(FormalParameterDeclaration *Param)
            : Expr(EK_Designator, Param->getType(), false), Var(Param) {}

        void addSelector(Selector *Sel) {
            Selectors.push_back(Sel);
            setType(Sel->getType());
        }

        Decl *getDecl() { return Var; }
        const SelectorList &getSelectors() const {
            return Selectors;
        }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Designator;
        }
    };

    class VariableAccess : public Expr {
        Decl *Var;

    public:
        VariableAccess(VariableDeclaration *Var)
            : Expr(EK_Var, Var->getType(), false), Var(Var) {}
        VariableAccess(FormalParameterDeclaration *Param)
            : Expr(EK_Var, Param->getType(), false), Var(Param) {}

        Decl *getDecl() { return Var; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Var;
        }
    };

    class ConstantAccess : public Expr {
        ConstantDeclaration *Const;

    public:
        ConstantAccess(ConstantDeclaration *Const)
            : Expr(EK_Const, Const->getExpr()->getType(), true), Const(Const) {}

        ConstantDeclaration *getDcl() { return Const; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Const;
        }
    };

    class FunctionCallExpr : public Expr {
        ProcedureDeclaration *Proc;
        ExprList Params;

    public:
        FunctionCallExpr(ProcedureDeclaration *Proc,
                         ExprList Params)
            : Expr(EK_Func, Proc->getRetType(), false), Proc(Proc), Params(Params) {}

        ProcedureDeclaration *getDecl() { return Proc; }
        const ExprList &getParams() { return Params; }

        static bool classof(const Expr *E) {
            return E->getKind() == EK_Func;
        }
    };

    class Stmt {
    public:
        enum StmtKind {
            SK_Assign,
            SK_ProcCall,
            SK_If,
            SK_While,
            SK_Return
        };

    private:
        const StmtKind Kind;

    protected:
        Stmt(StmtKind Kind): Kind(Kind) {}

    public:
        StmtKind getKind() const { return Kind; }
    };

    class AssignmentStatement : public Stmt {
        Designator *Var;
        Expr *E;

    public:
        AssignmentStatement(Designator *Var, Expr *E)
                : Stmt(SK_Assign), Var(Var), E(E) {}

        Designator *getVar() { return Var; }
        Expr *getExpr() { return E; }

        static bool classof(const Stmt *S) {
            return S->getKind() == SK_Assign;
        }
    };

    class ProcedureCallStatement : public Stmt {
        ProcedureDeclaration *Proc;
        ExprList Params;

    public:
        ProcedureCallStatement(ProcedureDeclaration *Proc,
                               ExprList &Params)
                : Stmt(SK_ProcCall), Proc(Proc), Params(Params) {}

        ProcedureDeclaration *getProc() { return Proc; }
        const ExprList &getParams() { return Params; }

        static bool classof(const Stmt *S) {
            return S->getKind() == SK_ProcCall;
        }
    };

    class IfStatement : public Stmt {
        Expr *Cond;
        StmtList IfStmts;
        StmtList ElseStmts;

    public:
        IfStatement(Expr *Cond, StmtList &IfStmts,
                    StmtList &ElseStmts)
                : Stmt(SK_If), Cond(Cond), IfStmts(IfStmts),
                  ElseStmts(ElseStmts) {}

        Expr *getCond() { return Cond; }
        const StmtList &getIfStmts() { return IfStmts; }
        const StmtList &getElseStmts() { return ElseStmts; }

        static bool classof(const Stmt *S) {
            return S->getKind() == SK_If;
        }
    };

    class WhileStatement : public Stmt {
        Expr *Cond;
        StmtList Stmts;

    public:
        WhileStatement(Expr *Cond, StmtList &Stmts)
                : Stmt(SK_While), Cond(Cond), Stmts(Stmts) {}

        Expr *getCond() { return Cond; }
        const StmtList &getWhileStmts() { return Stmts; }

        static bool classof(const Stmt *S) {
            return S->getKind() == SK_While;
        }
    };

    class ReturnStatement : public Stmt {
        Expr *RetVal;

    public:
        ReturnStatement(Expr *RetVal)
                : Stmt(SK_Return), RetVal(RetVal) {}

        Expr *getRetVal() { return RetVal; }

        static bool classof(const Stmt *S) {
            return S->getKind() == SK_Return;
        }
    };

}

#endif //TINYLANG_AST_H
