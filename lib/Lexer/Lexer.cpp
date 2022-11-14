//
// Created by kingkiller on 2022/11/7.
//
#include "tinylang/Lexer/Lexer.h"

using namespace tinylang;
using namespace llvm;

void KeywordFilter::addKeyword(llvm::StringRef Keyword, tok::TokenKind TokenCode) {
    HashTable.insert(std::make_pair(Keyword, TokenCode));
}


// 通过宏来添加关键字定义到hashtable中，其中#表示字符串引用(即将NAME参数变为字符串传入，##表示拼接(非字符串拼接)
void KeywordFilter::addKeywords() {
#define KEYWORD(NAME, FLAGS)                        \
    addKeyword(StringRef(#NAME), tok::kw_##NAME);
#include "tinylang/Basic/TokenKinds.def"
}

tok::TokenKind KeywordFilter::getKeyword(llvm::StringRef Name, tok::TokenKind DefaultTokenCode) {
    auto result = HashTable.find(Name);
    if (result != HashTable.end())
        return result->second;
    return DefaultTokenCode;
}

namespace charinfo {
    LLVM_READNONE inline bool isASCII(char ch) {
        return static_cast<unsigned char>(ch) <= 127;
    }

    // 检测是否是垂直空格(换行)
    LLVM_READNONE inline bool isVerticalWhitespace(char ch) {
        return isASCII(ch) && (ch == '\r' || ch == '\n');
    }

    // 检测是否是水平空格
    LLVM_READNONE inline bool isHorizontalWhitespace(char ch) {
        return isASCII(ch) && (ch == ' ' || ch == '\t' || ch == '\f'|| ch == '\v');
    }

    LLVM_READNONE inline bool isWhitespace(char ch) {
        return isHorizontalWhitespace(ch) || isVerticalWhitespace(ch);
    }

    LLVM_READNONE inline bool isDigit(char ch) {
        return isASCII(ch) && ch >= '0' && ch <= '9';
    }

    // 检测是否是十六进制数字
    LLVM_READNONE inline bool isHexDigit(char ch) {
        return isASCII(ch) && (isDigit(ch) || (ch >= 'A' && ch <= 'F'));
    }

    // 标识符可以用下划线开头，字母[_a-zA-Z]但不能用数字开头
    LLVM_READNONE inline bool isIdentifierHead(char ch) {
        return isASCII(ch) && (ch == '_' || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
    }

    // 标识符可以用数字结尾
    LLVM_READNONE inline bool isIdentifierBody(char ch) {
        return isIdentifierHead(ch) || isDigit(ch);
    }

}

Lexer::Lexer(SourceMgr &SrcMgr, DiagnosticsEngine &Diags)
    : SrcMgr(SrcMgr), Diags(Diags) {
    CurBuffer = SrcMgr.getMainFileID();
    CurBuf = SrcMgr.getMemoryBuffer(CurBuffer)->getBuffer();
    CurPtr = CurBuf.begin();
    Keywords.addKeywords();
}

void Lexer::next(Token &result) {
    while (*CurPtr && charinfo::isWhitespace(*CurPtr))
        ++CurPtr;
    if (!*CurPtr) {
        result.setKind(tok::eof);
        return;
    }
    if (charinfo::isIdentifierHead(*CurPtr)) {
        identifier(result);
        return;
    } else if (charinfo::isDigit(*CurPtr)) {
        number(result);
        return;
    } else if (*CurPtr == '"' || *CurPtr == '\'') {
        string(result);
        return;
    } else {
        switch (*CurPtr) {
#define CASE(ch, tok)                           \
        case ch:                                \
            formToken(result, CurPtr + 1, tok); \
            break
            CASE('=', tok::equal);
            CASE('#', tok::hash);
            CASE('+', tok::plus);
            CASE('-', tok::minus);
            CASE('*', tok::star);
            CASE('/', tok::slash);
            CASE(',', tok::comma);
            CASE('.', tok::period);
            CASE(';', tok::semi);
            CASE(')', tok::r_paren);
#undef CASE
            case '(':
                if (*(CurPtr + 1) == '*') {
                    comment();
                    next(result);
                } else {
                    formToken(result, CurPtr + 1, tok::l_paren);
                }
                break;
            case ':':
                if (*(CurPtr + 1) == '=')
                    formToken(result, CurPtr + 2, tok::colonequal);
                else
                    formToken(result, CurPtr + 1, tok::colon);
                break;
            case '<':
                if (*(CurPtr + 1) == '=')
                    formToken(result, CurPtr + 2, tok::lessequal);
                else
                    formToken(result, CurPtr + 1, tok::less);
                break;
            case '>':
                if (*(CurPtr + 1) == '=')
                    formToken(result, CurPtr + 2, tok::greaterequal);
                else
                    formToken(result, CurPtr + 1, tok::greater);
                break;
            default:
                result.setKind(tok::unknown);
        }
        return;
    }
}

void Lexer::identifier(Token &result) {
    const char *Start = CurPtr;
    const char *End = CurPtr + 1;
    while (charinfo::isIdentifierBody(*End))
        ++End;
    StringRef Name(Start, End - Start);
    formToken(result, End, Keywords.getKeyword(Name, tok::identifier));
}

void Lexer::number(Token &result) {
    const char *End = CurPtr + 1;
    tok::TokenKind Kind = tok::unknown;
    bool IsHex = false;
    while (*End) {
        if (!charinfo::isHexDigit(*End))
            break;
        if (!charinfo::isDigit(*End))
            IsHex = true;
        ++End;
    }
    switch (*End) {
        case 'H': /* hex number */
            Kind = tok::integer_literal;
            ++End;
            break;
        default:
            if (IsHex)
                Diags.report(getLoc(), diag::err_hex_digit_in_decimal);
            Kind = tok::integer_literal;
            break;
    }
    formToken(result, End, Kind);
}

void Lexer::string(Token &result) {
    const char *Start = CurPtr;
    const char *End = CurPtr + 1;
    while (*End && *End != *Start && !charinfo::isVerticalWhitespace(*End))
        ++End;
    if (charinfo::isVerticalWhitespace(*End)) {
        Diags.report(getLoc(), diag::err_unterminated_char_or_string);
    }
    formToken(result, End + 1, tok::string_literal);
}

void Lexer::comment() {
    const char *End = CurPtr + 2;
    unsigned Level = 1;
    while (*End && Level) {
        // Check for nested comment.
        if (*End == '(' && *(End + 1) == '*') {
            End += 2;
            Level++;
        }
        // Check for end of comment
        else if (*End == '*' && *(End + 1) == ')') {
            End += 2;
            Level++;
        }
        // Check for end of comment
        else if (*End == '*' && *(End + 1) == ')') {
            End += 2;
            Level--;
        } else
            ++End;
    }
    if (!*End)
        Diags.report(getLoc(), diag::err_unterminated_block_comment);
    CurPtr = End;
}

void Lexer::formToken(Token &result, const char *TokEnd, tok::TokenKind Kind) {
    result.Ptr = CurPtr;
    result.Length = TokEnd - CurPtr;
    result.Kind = Kind;
    CurPtr = TokEnd;
}



