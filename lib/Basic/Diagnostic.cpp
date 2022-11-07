//
// Created by BY210033 on 2022/11/7.
//
#include "tinylang/Basic/Diagnostic.h"

using namespace tinylang;
using namespace llvm;

namespace {

    const char *DiagnosticText[] = {
#define DIAG(ID, Level, Msg) Msg,
#include "tinylang/Basic/Diagnostic.def"
    };

    SourceMgr::DiagKind DiagnosticKind[] = {
#define DIAG(ID, Level, Msg) SourceMgr::DK_##Level,
#include "tinylang/Basic/Diagnostic.def"
    };

}