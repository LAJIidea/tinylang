//
// Created by kingkiller on 2022/11/7.
//

#ifndef TINYLANG_TOKENKINDS_H
#define TINYLANG_TOKENKINDS_H

#include <llvm/Support/Compiler.h>

namespace tinylang {

    // 将Token定义在tok命名空间中，这样就包含了全部的令牌，除此之外我们还可以直接在枚举类中定义Token
    namespace tok {
        enum TokenKind : unsigned short {
#define TOK(ID) ID,
#include "TokenKinds.def"
            NUM_TOKENS
        };

        const char *getTokenName(TokenKind Kind) LLVM_READNONE;

        const char *
        getPunctuatorSpelling(TokenKind Kind) LLVM_READNONE;

        const char *
        getKeywordSpelling(TokenKind Kind) LLVM_READNONE;

    } // namespace tok
} // namespace tinylang

#endif //TINYLANG_TOKENKINDS_H
