//
// Created by BY210033 on 2022/11/9.
//
#include "tinylang/Sema/Scope.h"
#include "tinylang/AST/AST.h"

using namespace tinylang;
using namespace llvm;

// 插入Declaration， 注意StringMap::insert()方法并不会覆盖现有规则条目。是否会更新成员，依据返回值
// 0 不会更新 1会更新（包括插入新值）
bool Scope::insert(Decl *Declaration) {
    return Symbols.insert(std::pair<StringRef, Decl*>(
            Declaration->getName(), Declaration
            )).second;
}

// 搜索当前作用域的declaration， 当前没有找到会依次搜索父级作用域
Decl *Scope::lookup(llvm::StringRef Name) {
    Scope *S = this;
    while (S) {
        StringMap<Decl *>::const_iterator  I =
                S->Symbols.find(Name);
        if (I != S->Symbols.end())
            return I->second;
        S = S->getParent();
    }
    return nullptr;
}


