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
    ProcedureDeclaration *D;
    FormalParamList  Params;
    Decl *RetType = nullptr;
    DeclList Decls;
    StmtList Stmts;
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
    advance();
    if (parseBlock(Decls, Stmts))
        goto _error;
    if (expect(tok::identifier))
        goto _error;
    // todo SemaOnProcedureDeclaration

    ParentDecls.push_back(D);
    advance();
    return false;
_error:
    while (!Tok.is(tok::semi)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseFormalParameters(FormalParamList &Params, Decl *&RetType) {
    if (consume(tok::l_paren))
        goto _error;
    if (Tok.isOneOf(tok::kw_VAR, tok::identifier)) {
        if (parseFormalParameter(Params))
            goto _error;
    }
    if (consume(tok::l_paren))
        goto _error;
    if (Tok.is(tok::colon)) {
        advance();
        if (parseQualident(RetType))
            goto _error;
    }
    return false;
_error:
    while (!Tok.is(tok::semi)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseFormalParameterList(FormalParamList &Params) {
    if (parseFormalParameter(Params))
        goto _error;
    while (Tok.is(tok::semi)) {
        advance();
        if (parseFormalParameter(Params))
            goto _error;
    }
    return false;
_error:
    while (!Tok.is(tok::r_paren)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseFormalParameter(FormalParamList &Params) {
    IdentList Ids;
    Decl *D;
    bool IsVar = false;
    if (Tok.is(tok::kw_VAR)) {
        IsVar = true;
        advance();
    }
    if (parseIdentList(Ids))
        goto _error;
    if (consume(tok::colon))
        goto _error;
    if (parseQualident(D))
        goto _error;
    // todo SemaOnFormalParameterDeclaration
    return false;
_error:
    while (!Tok.isOneOf(tok::r_paren, tok::semi)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseStatementSequence(StmtList &Stmts) {
    if (parseStatement(Stmts))
        goto _error;
    while (Tok.is(tok::semi)) {
        advance();
        if (parseStatement(Stmts))
            goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::kw_ELSE, tok::kw_END)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseStatement(StmtList &Stmts) {
    if (Tok.is(tok::identifier)) {
        Decl *D;
        Expr *E = nullptr;
        SMLoc Loc = Tok.getLocation();
        if (parseQualident(D))
            goto _error;
        if (Tok.is(tok::colonequal)) {
            advance();
            if (parseExpression(E))
                goto _error;
            // todo SemaOnAssignment
        } else if (Tok.is(tok::l_paren)) {
            ExprList Exprs;
            if (Tok.is(tok::l_paren)) {
                advance();
                if (Tok.isOneOf(tok::l_paren, tok::plus,
                                tok::minus, tok::kw_NOT,
                                tok::identifier, tok::integer_literal)) {
                    if (parseExpList(Exprs))
                        goto _error;
                }
                if (consume(tok::r_paren))
                    goto _error;
            }
            // todo SemaOnProCall
        }
    } else if (Tok.is(tok::kw_IF)) {
        if (parseIfStatement(Stmts))
            goto _error;
    } else if (Tok.is(tok::kw_IF)) {
        if (parseIfStatement(Stmts))
            goto _error;
    } else if (Tok.is(tok::kw_WHILE)) {
        if (parseIfStatement(Stmts))
            goto _error;
    } else if (Tok.is(tok::kw_RETURN)) {
        if (parseReturnStatement(Stmts))
            goto _error;
    } else {
        goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::semi, tok::kw_ELSE, tok::kw_END)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseIfStatement(StmtList &Stmts) {
    Expr *E = nullptr;
    StmtList IfStmts, ElseStmts;
    SMLoc Loc = Tok.getLocation();
    if (consume(tok::kw_IF))
        goto _error;
    if (parseExpression(E))
        goto _error;
    if (consume(tok::kw_THEN))
        goto _error;
    if (parseStatementSequence(IfStmts))
        goto _error;
    if (Tok.is(tok::kw_ELSE)) {
        advance();
        if (parseStatementSequence(ElseStmts))
            goto _error;
    }
    if (expect(tok::kw_END))
        goto _error;
    // todo SemaOnIfStatement

    advance();
    return false;
_error:
    while (!Tok.isOneOf(tok::semi, tok::kw_ELSE, tok::kw_END)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseWhileStatement(StmtList &Stmts) {
    Expr *E = nullptr;
    StmtList WhileStmts;
    SMLoc Loc = Tok.getLocation();
    if (consume(tok::kw_WHILE))
        goto _error;
    if (parseExpression(E))
        goto _error;
    if (consume(tok::kw_DO))
        goto _error;
    if (parseStatementSequence(WhileStmts))
        goto _error;
    // todo SemaOnWhileStatement
    advance();
    return false;
_error:
    while (!Tok.isOneOf(tok::semi, tok::kw_ELSE, tok::kw_END)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseReturnStatement(StmtList &Stmts) {
    Expr *E = nullptr;
    SMLoc Loc = Tok.getLocation();
    if (consume(tok::kw_RETURN))
        goto _error;
    if (Tok.isOneOf(tok::l_paren, tok::plus, tok::minus,
                    tok::kw_NOT, tok::identifier,
                    tok::integer_literal)) {
        if (parseExpression(E))
            goto _error;
    }
    // todo SemaOnReturnStatement
    return false;
_error:
    while (!Tok.isOneOf(tok::semi, tok::kw_ELSE, tok::kw_END)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseExpList(ExprList &Exprs) {
    Expr *E = nullptr;
    if (parseExpression(E))
        goto _error;
    if (E)
        Exprs.push_back(E);
    while (Tok.is(tok::comma)) {
        E = nullptr;
        advance();
        if (parseExpression(E))
            goto _error;
        if (E)
            Exprs.push_back(E);
    }
    return false;
_error:
    while (!Tok.is(tok::r_paren)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseExpression(Expr *&E) {
    if (parseSimpleExpression(E))
        goto _error;
    if (Tok.isOneOf(tok::hash, tok::less, tok::lessequal,
                    tok::equal, tok::greater,
                    tok::greaterequal)) {
        OperatorInfo Op;
        Expr *Right = nullptr;
        if (parseRelation(Op))
            goto _error;
        if (parseSimpleExpression(Right))
            goto _error;
        // todo SemaOnExpression
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::r_paren, tok::comma, tok::semi,
                        tok::kw_DO, tok::kw_ELSE, tok::kw_END,
                        tok::kw_THEN)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseRelation(OperatorInfo &Op) {
    if (Tok.is(tok::equal)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::hash)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::less)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::lessequal)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::greater)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::greaterequal)) {
        Op = fromTok(Tok);
        advance();
    } else {
        /* ERROR */
        goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::l_paren, tok::plus, tok::minus,
                        tok::kw_NOT, tok::identifier,
                        tok::integer_literal)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseSimpleExpression(Expr *&E) {
    OperatorInfo PrefixOp;
    Expr *Right = nullptr;
    if (Tok.isOneOf(tok::plus, tok::minus)) {
        if (Tok.is(tok::plus)) {
            PrefixOp = fromTok(Tok);
            advance();
        }
    }
    if (parseTerm(E))
        goto _error;
    while (Tok.isOneOf(tok::plus, tok::minus, tok::kw_OR)) {
        OperatorInfo Op;
        Right = nullptr;
        if (parseAddOperator(Op))
            goto _error;
        if (parseTerm(Right))
            goto _error;
        // todo SemaOnSimpleExpression
    }
    if (!PrefixOp.isUnspecified())
        // todo SemaOnPrefixExpression
        E = nullptr;
    return false;
_error:
    while (!Tok.isOneOf(tok::hash, tok::r_paren, tok::comma, tok::semi,
                        tok::less, tok::lessequal, tok::equal, tok::greater,
                        tok::greaterequal, tok::kw_DO, tok::kw_ELSE,
                        tok::kw_END, tok::kw_THEN)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseAddOperator(OperatorInfo &Op) {
    if (Tok.is(tok::plus)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::minus)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::kw_OR)) {
        Op = fromTok(Tok);
        advance();
    } else {
        goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::l_paren, tok::kw_NOT,
                         tok::identifier, tok::integer_literal)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseTerm(Expr *&E) {
    if (parseFactor(E))
        goto _error;
    while (Tok.isOneOf(tok::star, tok::slash, tok::kw_AND,
                       tok::kw_DIV, tok::kw_MOD)) {
        OperatorInfo Op;
        Expr *Right = nullptr;
        if (parseMulOperator(Op))
            goto _error;
        if (parseFactor(Right))
            goto _error;
        // todo SemaOnTerm
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::hash, tok::r_paren, tok::plus,
                        tok::comma, tok::minus, tok::semi,
                        tok::lessequal, tok::lessequal, tok::equal,
                        tok::greater, tok::greaterequal,
                        tok::kw_DO, tok::kw_ELSE, tok::kw_END,
                        tok::kw_OR, tok::kw_THEN)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseMulOperator(OperatorInfo &Op) {
    if (Tok.is(tok::star)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::slash)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::kw_MOD)) {
        Op = fromTok(Tok);
        advance();
    } else if (Tok.is(tok::kw_AND)) {
        Op = fromTok(Tok);
        advance();
    } else {
        goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::l_paren, tok::kw_NOT,
                        tok::identifier, tok::integer_literal)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseFactor(Expr *&E) {
    if (Tok.is(tok::integer_literal)) {
        // todo SemaOnIntegerLiteral
        advance();
    } else if (Tok.is(tok::identifier)) {
        Decl *D;
        ExprList Exprs;
        if (parseQualident(D))
            goto _error;
        if (Tok.is(tok::l_paren)) {
            advance();
            if (Tok.isOneOf(tok::l_paren, tok::plus, tok::minus,
                            tok::kw_NOT, tok::identifier,
                            tok::identifier)) {
                if (parseExpList(Exprs))
                    goto _error;
            }
            if (expect(tok::r_paren))
                goto _error;
            // todo SemaOnFunctionCall
            advance();
        } else if (Tok.isOneOf(
                    tok::hash, tok::r_paren, tok::star,
                    tok::plus, tok::comma, tok::minus,
                    tok::slash, tok::semi, tok::less,
                    tok::lessequal, tok::equal,
                    tok::greater, tok::greaterequal,
                    tok::kw_ELSE, tok::kw_END, tok::kw_MOD,
                    tok::kw_OR, tok::kw_THEN
                )) {
            // todo SemaOnVariable
        }
    } else if (Tok.is(tok::l_paren)) {
        advance();
        if (parseExpression(E))
            goto _error;
        if (consume(tok::r_paren))
            goto _error;
    } else if (Tok.is(tok::kw_NOT)) {
        OperatorInfo Op = fromTok(Tok);
        advance();
        if (parseFactor(E))
            goto _error;
        // todo SemaOnPrefixExpression
    } else {
        goto _error;
    }
    return false;
_error:
    while (!Tok.isOneOf(
            tok::hash, tok::r_paren, tok::star, tok::plus,
            tok::comma, tok::minus, tok::slash, tok::semi,
            tok::less, tok::lessequal, tok::equal, tok::greater,
            tok::greaterequal, tok::kw_AND, tok::kw_DIV,
            tok::kw_DO, tok::kw_ELSE, tok::kw_END, tok::kw_MOD,
            tok::kw_OR, tok::kw_THEN)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseQualident(Decl *&D) {
    D = nullptr;
    if (expect(tok::identifier))
        goto _error;
    // todo SemaOnQualIdentPart
    advance();
    while (Tok.is(tok::period) &&
           (isa<ModuleDeclaration>(D))) {
        advance();
        if (expect(tok::identifier))
            goto _error;
        // todo SemaOnQualIdentPart
    }
    return false;
_error:
    while (!Tok.isOneOf(
            tok::hash, tok::l_paren, tok::r_paren, tok::star,
            tok::plus, tok::comma, tok::minus, tok::slash,
            tok::colonequal, tok::semi, tok::less, tok::lessequal,
            tok::equal, tok::greater, tok::greaterequal,
            tok::kw_AND, tok::kw_DIV, tok::kw_DO, tok::kw_ELSE,
            tok::kw_END, tok::kw_MOD, tok::kw_OR, tok::kw_THEN)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
    return false;
}

bool Parser::parseIdentList(IdentList &Ids) {
    if (expect(tok::identifier))
        goto _error;
    Ids.push_back(std::pair<SMLoc, StringRef>(
            Tok.getLocation(), Tok.getIdentifier()
            ));
    advance();
    while (Tok.is(tok::comma)) {
        advance();
        if (expect(tok::identifier))
            goto _error;
        Ids.push_back(std::pair<SMLoc, StringRef>(
                Tok.getLocation(), Tok.getIdentifier()
                ));
        advance();
    }
    return false;
_error:
    while (!Tok.isOneOf(tok::colon, tok::semi)) {
        advance();
        if (Tok.is(tok::eof))
            return true;
    }
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
