//
// Created by BY210033 on 2022/11/15.
//

#ifndef TINYLANG_CGMODULE_H
#define TINYLANG_CGMODULE_H

#include "tinylang/AST/AST.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace tinylang {

    class CGModule {
        llvm::Module *M;

        ModuleDeclaration *Mod;

        llvm::DenseMap<Decl *, llvm::GlobalObject *> Globals;

    public:
        llvm::Type *VoidTy;
        llvm::Type *Int1Ty;
        llvm::Type *Int32Ty;
        llvm::Type *Int64Ty;
        llvm::Constant *Int32Zero;

        CGModule(llvm::Module *M) : M(M) { initialize(); }
        void initialize();

        llvm::LLVMContext &getLLVMCtx() { return M->getContext(); }
        llvm::Module *getModule() { return M; }
        ModuleDeclaration *getModuleDeclaration() { return Mod; }

        llvm::Type *convertType(TypeDeclaration *Ty);
        std::string  mangleName(Decl *D);

        llvm::GlobalObject *getGlobal(Decl *);

        void run(ModuleDeclaration *Mod);
    };

} // namespace tinylang

#endif //TINYLANG_CGMODULE_H
