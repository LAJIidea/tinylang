//
// Created by BY210033 on 2022/11/21.
//

#ifndef TINYLANG_ASTCONTEXT_H
#define TINYLANG_ASTCONTEXT_H

#include <llvm/Support/SourceMgr.h>

namespace tinylang {
    class ASTContext {
        llvm::SourceMgr &SrcMgr;
        llvm::StringRef Filename;

    public:
        ASTContext(llvm::SourceMgr &SrcMgr, llvm::StringRef Filename)
            : SrcMgr(SrcMgr), Filename(Filename) {}

        llvm::StringRef getFilename() { return Filename; }

        llvm::SourceMgr &getSourceMgr() { return SrcMgr; }
        const llvm::SourceMgr &getSourceMgr() const {
            return SrcMgr;
        }
    };
}

#endif //TINYLANG_ASTCONTEXT_H
