//
// Created by BY210033 on 2022/11/7.
//

#ifndef TINYLANG_DIAGNOSTIC_H
#define TINYLANG_DIAGNOSTIC_H

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/SMLoc.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

#include <utility>

namespace tinylang {

    // 使用Diagnostic.def来定义枚举，为了不污染全局命名空间，需要使用diag命名空间
    namespace diag {
        enum {
#define DIAG(ID, Level, Msg) ID,
#include "tinylang/Basic/Diagnostic.def"
        };
    }

    /**
     * DiagnosticsEngine类使用SourceMgr实例通过report()发出消息，消息可以有参数。
     * 为了实现这个功能，必须要使用LLVM的可变格式支持，在静态方法的帮助下检索消息文本和严重性级别，
     * 除此之外还需要计算发出错误消息的数量
     */
    class DiagnosticsEngine {
        // 返回字符串消息
        static const char *getDiagnosticText(unsigned DiagID);
        // 返回错误级别
        static llvm::SourceMgr::DiagKind
        getDiagnosticKind(unsigned DiagID);

        llvm::SourceMgr &SrcMgr;
        unsigned NumErrors;

    public:
        DiagnosticsEngine(llvm::SourceMgr &SrcMgr)
            : SrcMgr(SrcMgr), NumErrors(0) {}

        unsigned nunErrors() { return NumErrors; }

        // 因为消息可以有可变数量的参数, 所以C++中的解决方案是使用可变参数模板。
        template<typename... Args>
        void report(llvm::SMLoc Loc, unsigned DiagID, Args &&... Arguments) {
            // 下面LLVM提供的format函数也是利用可变参数模板实现的
            std::string Msg = llvm::formatv(getDiagnosticText(DiagID), std::forward<Args>(Arguments)...)
                    .str();
            // 想要获得格式化的信息，我们只需要转发模板参数
            llvm::SourceMgr::DiagKind Kind = getDiagnosticKind(DiagID);
            SrcMgr.PrintMessage(Loc, Kind, Msg);
            NumErrors += (Kind == llvm::SourceMgr::DK_Error);
        }
    };

} // namespace tinylang

#endif //TINYLANG_DIAGNOSTIC_H
