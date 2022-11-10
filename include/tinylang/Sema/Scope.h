//
// Created by BY210033 on 2022/11/8.
//

#ifndef TINYLANG_SCOPE_H
#define TINYLANG_SCOPE_H

#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>

/**
 * 在tinylang中，只有模块和过程才能打开一个新的作用域，而名称才是作用域的关键。
 * 因此，作用域可以实现为从名称到其声明的映射，只有在新名称不存在的情况下才能插入新名称。
 * 对于查找，封闭或父作用域也必须已知。
 */
namespace tinylang {

    class Decl;

    class Scope {
        Scope *Parent;
        llvm::StringMap<Decl *> Symbols;

    public:
        Scope(Scope *Parent = nullptr) : Parent(Parent) {}

        bool insert(Decl *Declaration);
        Decl *lookup(llvm::StringRef Name);

        Scope *getParent() { return Parent; }

    };

} // namespace tinylang

#endif //TINYLANG_SCOPE_H
