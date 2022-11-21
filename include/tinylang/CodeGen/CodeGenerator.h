//
// Created by BY210033 on 2022/11/15.
//

#ifndef TINYLANG_CODEGENERATOR_H
#define TINYLANG_CODEGENERATOR_H

#include "tinylang/AST/AST.h"
#include "tinylang/AST/ASTContext.h"

#include <llvm/Target/TargetMachine.h>

#include <string>

namespace tinylang {

    class CodeGenerator {
        llvm::LLVMContext &Ctx;
        ASTContext &ASTCtx;
        llvm::TargetMachine *TM;
        ModuleDeclaration *CM;

    protected:
        CodeGenerator(llvm::LLVMContext &Ctx, ASTContext &ASTCtx, llvm::TargetMachine *TM)
            : Ctx(Ctx), ASTCtx(ASTCtx), TM(TM), CM(nullptr) {}

    public:
        static CodeGenerator *create(llvm::LLVMContext &Ctx, ASTContext &ASTCtx, llvm::TargetMachine *TM);

        std::unique_ptr<llvm::Module> run(ModuleDeclaration *CM, std::string FileName);
    };

}

#endif //TINYLANG_CODEGENERATOR_H
