//
// Created by BY210033 on 2022/11/8.
//
#include "tinylang/Parser/Parser.h"

using namespace tinylang;
using namespace llvm;

namespace {
    OperatorInfo fromTok(Token tok)
    {
        return OperatorInfo(tok.getLocation(), tok.getKind());
    }
} // namespace

bool Parser::parseCompilationUnit(ModuleDeclaration *&D) {
    DeclList decls;
    StmtList stmts;
    if (consume(tok::kw_MODULE))
        goto _error;
    if (expect(tok::identifier))
        goto _error;
    // todo SemaOnModuleDeclaration
    advance();
    if (consume(tok::semi))
        goto _error;
    while (Tok.isOneOf(tok::kw_FROM, tok::kw_IMPORT)) {
        if (parseImport())
            goto _error;
    }
    if (parseBlock(decls, stmts))
        goto _error;
    if (expect(tok::identifier))
        goto _error;
    // todo SemaModuleDeclaration
    advance();
    if (consume(tok::period))
        goto _error;
    return false;

_error:
    while (!Tok.is(tok::eof)) {
        advance();
    }
    return false;
}

bool Parser::parseImport() {
    IdentList Ids;
    StringRef ModuleName;
    if (Tok.is(tok::kw_FROM)) {
        advance();
        if (expect(tok::identifier))
            goto _error;
        ModuleName = Tok.getIdentifier();
        advance();
    }
    if (consume(tok::kw_IMPORT))
        goto _error;
    if (parseIdentList(Ids))
        goto _error;
    if (expect(tok::semi))
        goto _error;
    // todo SemaOnImport
    advance();
    return false;
_error:
    while (!Tok.isOneOf(tok::kw_BEGIN, tok::kw_CONST,
                        tok::kw_END, tok::kw_FROM,
                        tok::kw_IMPORT, tok::kw_PROCEDURE,
                        tok::kw_VAR)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseBlock(DeclList &Decls, StmtList &Stmts) {
    while (Tok.isOneOf(tok::kw_CONST, tok::kw_PROCEDURE, tok::kw_VAR)) {
        if (parseDeclaration(Decls))
            goto _error;
    }
    if (Tok.is(tok::kw_BEGIN)) {
        advance();
        if (parseStatementSequence(Stmts))
            goto _error;
    }
    if (consume(tok::kw_END))
        goto _error;
    return false;
_error:
    while (!Tok.is(tok::identifier)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseDeclaration(DeclList &Decls) {
    if (Tok.is(tok::kw_CONST)) {
        advance();
        while (Tok.is(tok::identifier)) {
            if (parseConstantDeclaration(Decls))
                goto _error;
            if (consume(tok::semi))
                goto _error;
        }
    } else if (Tok.is(tok::kw_VAR)) {
        advance();
        while (Tok.is(tok::identifier)) {
            if (parseVariableDeclaration(Decls))
                goto _error;
            if (consume(tok::semi))
                goto _error;
        }
    } else if (Tok.is(tok::kw_PROCEDURE)) {
        if (parseProcedureDeclaration(Decls))
            goto _error;
        if (consume(tok::semi))
            goto _error;
    } else {
        /*ERROR*/
        goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::kw_BEGIN, tok::kw_CONST,
                        tok::kw_END, tok::kw_PROCEDURE,
                        tok::kw_VAR)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseConstantDeclaration(DeclList &Decls) {
    StringRef Name;
    SMLoc Loc;
    Expr *E = nullptr;
    if (expect(tok::identifier))
        goto _error;
   Loc = Tok.getLocation();

    Name = Tok.getIdentifier();
    advance();
    if (expect(tok::equal))
        goto _error;
    advance();
    if (parseExpression(E))
        goto _error;
    // todo SemaOnConstantDeclaration
    return false;
_error:
    while (!Tok.is(tok::semi)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseVariableDeclaration(DeclList &Decls) {
    Decl *D;
    IdentList Ids;
    if (parseIdentList(Ids))
        goto _error;
    if (parseQualident(D))
        goto _error;
    // todo SemaOnVariableDeclaration
_error:
    while (!Tok.is(tok::semi)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseProcedureDeclaration(DeclList &ParentDecls) {
    FormalParamList  Params;
    Decl *RetType = nullptr;
    if (consume(tok::kw_PROCEDURE))
        goto _error;
    if (expect(tok::identifier))
        goto _error;
    // todo SemaOnProcedureDeclaration
    advance();
    if (Tok.is(tok::l_paren)) {
        if (parseFormalParameters(Params, RetType))
            goto _error;
    }
    // todo SemaOnProcedureHeading
    if (expect(tok::semi))
        goto _error;
_error:
    return false;
}

bool Parser::parseFormalParameters(FormalParamList &Params, Decl *&RetType) {
    return false;
}

bool Parser::parseFormalParameterList(FormalParamList &Params) {
    return false;
}

bool Parser::parseFormalParameter(FormalParamList &Params) {
    return false;
}

bool Parser::parseStatementSequence(StmtList &Stmts) {
    return false;
}

bool Parser::parseStatement(StmtList &Stmts) {
    return false;
}

bool Parser::parseIfStatement(StmtList &Stmts) {
    return false;
}

bool Parser::parseWhileStatement(StmtList &Stmts) {
    return false;
}

bool Parser::parseReturnStatement(StmtList &Stmts) {
    return false;
}

bool Parser::parseExpList(ExprList &Exprs) {
    return false;
}

bool Parser::parseExpression(Expr *&E) {
    return false;
}

bool Parser::parseRelation(OperatorInfo &Op) {
    return false;
}

bool Parser::parseSimpleExpression(Expr *&E) {
    return false;
}

bool Parser::parseAddOperator(OperatorInfo &Op) {
    return false;
}

bool Parser::parseTerm(Expr *&E) {
    return false;
}

bool Parser::parseMulOperator(OperatorInfo &Op) {
    return false;
}

bool Parser::parseFactor(Expr *&E) {
    return false;
}

bool Parser::parseQualident(Decl *&D) {
    return false;
}

bool Parser::parseIdentList(IdentList &Ids) {
    return false;
}

Parser::Parser(Lexer &Lex): Lex(Lex) {
    advance();
}

ModuleDeclaration *Parser::parse() {
    ModuleDeclaration *ModDecl = nullptr;
    parseCompilationUnit(ModDecl);
    return ModDecl;
}
