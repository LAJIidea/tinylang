//
// Created by kingkiller on 2022/11/7.
//

#ifndef TINYLANG_TOKEN_H
#define TINYLANG_TOKEN_H

#include "tinylang/Basic/TokenKinds.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/SMLoc.h>
#include <llvm/ADT/None.h>
#include <llvm/Support/Casting.h>

namespace tinylang {

    class Token {
        friend class Lexer;

        // token 的位置
        const char *Ptr;

        // size_t并非C/C++的内置类型，而是定义在基本库中的 unsigned int
        // token 的长度
        size_t Length;

        // token的类型
        tok::TokenKind Kind;

    public:
        // 获取枚举类型
        tok::TokenKind getKind() const { return Kind; }
        void setKind(tok::TokenKind K) { Kind = K; }

        // 获取token长度
        size_t getLength() const { return Length; }

        // SMLoc实例表示源在消息中的位置，它是使用指向令牌的指针创建
        llvm::SMLoc getLocation() const {
            return llvm::SMLoc::getFromPointer(Ptr);
        }

        // 断言token是否是特指的类型或之一
        bool is(tok::TokenKind K) const { return Kind == K; }
        bool isNot(tok::TokenKind K) const { return Kind != K; }
        bool isOneOf(tok::TokenKind K1, tok::TokenKind K2) const {
            return is(K1) || is(K2);
        }

        template<typename... Ts>
        bool isOneOf(tok::TokenKind K1, tok::TokenKind K2, Ts... Ks) const {
            return is(K1) || isOneOf(K2, Ks...);
        }

        // 允许访问标识符和数字文本
        llvm::StringRef getIdentifier() {
            assert(is(tok::identifier) && "Cannot get identifier of non-identifier");
            return llvm::StringRef(Ptr, Length);
        }

        llvm::StringRef getLiteralData() {
            assert(isOneOf(tok::integer_literal, tok::string_literal) && "Cannot get literal data of non-literal");
            return {Ptr, Length};
        }
    };
} // namespace tinylang

#endif //TINYLANG_TOKEN_H
