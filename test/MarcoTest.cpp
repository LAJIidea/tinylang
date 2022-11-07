//
// Created by BY210033 on 2022/11/7.
//
#include <iostream>
#include <string>
#include "tinylang/Basic/Diagnostic.h"

int main()
{
    const char *DiagnosticText[] = {
#define DIAG(ID, Level, Msg) Msg,
#include "tinylang/Basic/Diagnostic.def"
    };

    llvm::SourceMgr::DiagKind DiagnosticKind[] = {
#define DIAG(ID, Level, Msg) llvm::SourceMgr::DK_##Level,
#include "tinylang/Basic/Diagnostic.def"
    };


//    for (auto item : DiagnosticText) {
//        std::cout << item << std::endl;
//    }

    for (auto item : DiagnosticKind) {
        std::cout << item << std::endl;
    }

}