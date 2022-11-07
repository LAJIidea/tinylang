//
// Created by kingkiller on 2022/11/7.
//

#ifndef TINYLANG_TOKEN_H
#define TINYLANG_TOKEN_H

#include "tinylang/Basic/TokenKinds.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/SMLoc.h>

namespace tinylang {

    class Token {
        friend class Lexer;

        // token 的位置
        const char *Ptr;

        // size_t并非C/C++的内置类型，而是定义在基本库中的 unsigned int
        // token 的长度
        size_t Length;

        tok::TokenKind Kind;

    };
}

#endif //TINYLANG_TOKEN_H
