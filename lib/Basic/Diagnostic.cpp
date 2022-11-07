//
// Created by BY210033 on 2022/11/7.
//
#include "tinylang/Basic/Diagnostic.h"

using namespace tinylang;
using namespace llvm;

namespace {

    // 这里定义一个数组来保存文本消息
    const char *DiagnosticText[] = {
#define DIAG(ID, Level, Msg) Msg,
#include "tinylang/Basic/Diagnostic.def"
    };

    // 同样使用一个数组来保存错误等级
    SourceMgr::DiagKind DiagnosticKind[] = {
#define DIAG(ID, Level, Msg) SourceMgr::DK_##Level,
#include "tinylang/Basic/Diagnostic.def"
    };

} // namespace

// 获取错误文本和level的函数具体实现都只是简单对数组进行索引，返回对应数据
const char *DiagnosticsEngine::getDiagnosticText(unsigned int DiagID) {
    return DiagnosticText[DiagID];
}

llvm::SourceMgr::DiagKind DiagnosticsEngine::getDiagnosticKind(unsigned int DiagID) {
    return DiagnosticKind[DiagID];
}
