//
// Created by BY210033 on 2022/11/11.
//
#include "tinylang/Basic/Diagnostic.h"
#include "tinylang/Basic/Version.h"
#include "tinylang/Parser/Parser.h"

#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/raw_ostream.h>

using namespace tinylang;

int main(int argc, const char **argv_)
{
    llvm::InitLLVM X(argc, argv_);
    llvm::SmallVector<const char *, 256> argv(argv_ + 1, argv_ + argc);

    llvm::outs() << "Tinylang "
                 << getTinylangVersion() << "\n";

    for (const char *F : argv) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
            FileOrErr = llvm::MemoryBuffer::getFile(F);
        if (std::error_code BufferError = FileOrErr.getError()) {
            llvm::errs() << "Error reading " << F << ": "
                         << BufferError.message() << "\n";
            continue;
        }

        llvm::SourceMgr SrcMgr;
        DiagnosticsEngine Diags(SrcMgr);

        // Tell SrcMgr about this buffer, which is what the
        // parser will pick up.
        SrcMgr.AddNewSourceBuffer(std::move(*FileOrErr), llvm::SMLoc());

        auto lexer = Lexer(SrcMgr, Diags);
        auto sema = Sema(Diags);
        auto parser = Parser(lexer, sema);
        parser.parse();
    }

}
