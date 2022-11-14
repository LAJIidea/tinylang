//
// Created by BY210033 on 2022/11/7.
//
#include <iostream>
#include <string>
#include "tinylang/Basic/Diagnostic.h"

bool isASCII(char ch) {
    return static_cast<unsigned char>(ch) <= 127;
}

bool isHorizontalWhitespace(char ch) {
    return isASCII(ch) && (ch == ' ' || ch == '\t' || ch == '\f', ch == '\v');
}

bool isWhitespace(char  ch)
{

}

int main()
{
//    const char *DiagnosticText[] = {
//#define DIAG(ID, Level, Msg) Msg,
//#include "tinylang/Basic/Diagnostic.def"
//    };
//
//    llvm::SourceMgr::DiagKind DiagnosticKind[] = {
//#define DIAG(ID, Level, Msg) llvm::SourceMgr::DK_##Level,
//#include "tinylang/Basic/Diagnostic.def"
//    };
//
//
////    for (auto item : DiagnosticText) {
////        std::cout << item << std::endl;
////    }
//
//    for (auto item : DiagnosticKind) {
//        std::cout << item << std::endl;
//    }
    char s = ' ';
    char *p = &s;
    if (*p) {
        std::cout << true << std::endl;
    } else {
        std::cout << false << std::endl;
    }

    auto cs = static_cast<unsigned char >(*p);
    if (cs <= 127) {
        std::cout << true << std::endl;
    } else {
        std::cout << false << std::endl;
    }
    if (cs == ' ' || cs == '\t' || cs == '\f'|| cs == '\v') {
        std::cout << true << std::endl;
    } else {
        std::cout << false << std::endl;
    }
}