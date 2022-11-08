//
// Created by kingkiller on 2022/11/7.
//

#ifndef TINYLANG_LEXER_H
#define TINYLANG_LEXER_H

#include "Token.h"
#include "tinylang/Basic/Diagnostic.h"

#include <llvm/ADT/StringMap.h>

namespace tinylang {

    class KeywordFilter {

        // 当拥有很多关键字时，一个快速简单的方案是通过哈希表存储这些keyword
        // 存储这些关键字会在Lexer类实例化时完成，这种方法还可以支持不同级别的语言
        // 这里使用llvm::StringMap这一个llvm自带的哈希表
        llvm::StringMap<tok::TokenKind> HashTable;

        void addKeyword(llvm::StringRef Keyword, tok::TokenKind TokenCode);

    public:
        // 存储关键字
        void addKeywords();

        // 获取给定字符串的token类型，如果字符串不代表关键字，则返回默认值
        tok::TokenKind getKeyword(llvm::StringRef Name,
                                  tok::TokenKind DefaultTokenCode = tok::unknown);

    };

    class Lexer {
        llvm::SourceMgr &SrcMgr;
        DiagnosticsEngine &Diags;

        const char *CurPtr;
        llvm::StringRef CurBuf;

        // 这是由SourceMgr对象管理的当前缓冲索引
        unsigned CurBuffer = 0;

        KeywordFilter Keywords;

    public:
        Lexer(llvm::SourceMgr &SrcMgr, DiagnosticsEngine &Diags);

        DiagnosticsEngine &getDiagnostics() const {
            return Diags;
        }

        // 根据输入的令牌获取下一个token
        void next(Token &result);

        llvm::StringRef getBuffer() const { return CurBuf; }

    private:
        void identifier(Token &result);
        void number(Token &result);
        void string(Token &result);
        void comment();


        llvm::SMLoc getLoc() { return llvm::SMLoc::getFromPointer(CurPtr); }

        void formToken(Token &result, const char *TokEnd, tok::TokenKind Kind);

    };

} // namespace

#endif //TINYLANG_LEXER_H
