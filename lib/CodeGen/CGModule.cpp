//
// Created by BY210033 on 2022/11/15.
//
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/CommandLine.h>

#include "tinylang/CodeGen/CGModule.h"
#include "tinylang/CodeGen/CGProcedure.h"

using namespace tinylang;
using namespace llvm;

static llvm::cl::opt<bool>
        Debug("g", llvm::cl::desc("Generate debug information"), llvm::cl::init(false));

CGModule::CGModule(ASTContext &ASTCtx, llvm::Module *M)
    : ASTCtx(ASTCtx), M(M) {
    initialize();
}

void CGModule::initialize() {
    VoidTy = Type::getVoidTy(getLLVMCtx());
    Int1Ty = Type::getInt1Ty(getLLVMCtx());
    Int32Ty = Type::getInt32Ty(getLLVMCtx());
    Int64Ty = Type::getInt64Ty(getLLVMCtx());
    Int32Zero = ConstantInt::get(Int32Ty, 0, true);
}

llvm::Type *CGModule::convertType(TypeDeclaration *Ty) {
    if (Type *T = TypeCache[Ty])
        return T;

    if (isa<PervasiveTypeDeclaration>(Ty)) {
        if (Ty->getName() == "INTEGER")
            return Int64Ty;
        else if (Ty->getName() == "BOOLEAN")
            return Int1Ty;
    } else if (auto *AliasTy = dyn_cast<AliasTypeDeclaration>(Ty)) {
        Type *T = convertType(AliasTy->getType());
        return TypeCache[Ty] = T;
    } else if (auto *ArrayTy = dyn_cast<ArrayTypeDeclaration>(Ty)) {
        Type *Component = convertType(ArrayTy->getType());
        Expr *Nums = ArrayTy->getNums();
        uint64_t NumElements = 5; // todo Eval Nums
        Type *T = llvm::ArrayType::get(Component, NumElements);
        return TypeCache[Ty] = T;
    } else if (auto *RecordTy = dyn_cast<RecordTypeDeclaration>(Ty)) {
        SmallVector<Type *, 4> Elements;
        for (const auto &F : RecordTy->getFields()) {
            Elements.push_back(convertType(F.getType()));
        }
        Type *T = StructType::create(Elements, RecordTy->getName(), false);
        return TypeCache[Ty] = T;
    }
    report_fatal_error("Unsupported type");
}

// 函数(以及全局变量)有一个附加的链接样式。使用链接样式，我们定义了符号名的可见性，以及如果多个符号具有相同的名称一个发生什么.
// 最基本的链接样式是私有和外部可访问的。具有私有链接的符号仅在当前编译单元可见，而具有外部链接的符号则是全局可用
/*
 * 对于没有适当模块概念的C语言，这就足够了，但是对于模块语言，有更多的情况。假设一个Square模块提供了一个root函数。
 * 而Cube模块也提供一个root函数，如果函数是私有的没有问题，但如果函数可以导出，被其他模块调用，仅使用函数名是不够的，因为函数名不唯一
 * 解决方案是调整名称，使其具有全局唯一性，这叫命名修饰，如何做到这一点取决与语言的需求和特征。我们的例子中，基本思想是使用模块名和函数名的组合来创建一个全局唯一名。使
 * 用 Square.Root 作为名称看起来是一个简单的解决方案，但可能会导致汇编程序的问题，因为点
 * 可能有特殊的含义。我们不需要在名称组件之间使用分隔符，而是可以用名称组件的长度作为前
 * 缀:6Square4Root，从而获得类似的效果。这不是 LLVM 的合法标识符，但可以通过在整个名称前
 * 加上_t (t 代表 tinylang):_t6Square4Root 来解决这个问题。通过这种方式，我们可以为导出的符
 * 号，并创建唯一的名称
 */
std::string CGModule::mangleName(Decl *D) {
    std::string Mangled;
    SmallString<16> Tmp;
    while (D) {
        StringRef Name = D->getName();
        Tmp.clear();
        Tmp.append(llvm::itostr(Name.size()));
        Tmp.append(Name);
        Mangled.insert(0, Tmp.c_str());
        D = D->getEnclosingDecl();
    }
    Mangled.insert(0, "_t");
    return Mangled;
}

llvm::GlobalObject *CGModule::getGlobal(Decl *D) {
    return Globals[D];
}

void CGModule::run(ModuleDeclaration *Mod) {
    for (auto *Decl : Mod->getDecls()) {
        if (auto *Var = dyn_cast<VariableDeclaration>(Decl)) {
            GlobalVariable *V = new GlobalVariable(*M, convertType(Var->getType()), false,
                                                   GlobalVariable::PrivateLinkage, nullptr,
                                                   mangleName(Var));
            Globals[Var] = V;
        } else if (auto *Proc = dyn_cast<ProcedureDeclaration>(Decl)) {
            CGProcedure CGP(*this);
            CGP.run(Proc);
        }
    }
}
