//
// Created by BY210033 on 2022/11/11.
//
#include "tinylang/Basic/Diagnostic.h"
#include "tinylang/Basic/Version.h"
#include "tinylang/Parser/Parser.h"
#include "tinylang/CodeGen/CodeGenerator.h"

#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/WithColor.h>

using namespace tinylang;

static llvm::codegen::RegisterCodeGenFlags CGF;

static llvm::cl::list<std::string> InputFiles(llvm::cl::Positional, llvm::cl::desc("<input-files>"));

static llvm::cl::opt<std::string> MTriple("mtriple", llvm::cl::desc("Override target triple for module"));

static llvm::cl::opt<bool> EmitLLVM("emit-llvm", llvm::cl::desc("Emit IR code instead of assembler"),
                                           llvm::cl::init(false));

static const char *Head = "tinylang - Tinylang compiler";

void printVersion(llvm::raw_ostream &OS)
{
    OS << Head << " " << getTinylangVersion() << "\n";
    OS << " Default target: "
       << llvm::sys::getDefaultTargetTriple() << "\n";
    std::string CPU(llvm::sys::getHostCPUName());
    OS << " Host CPU: " << CPU << "\n";
    OS << "\n";
    OS.flush();
    llvm::TargetRegistry::printRegisteredTargetsForVersion(OS);
    exit(EXIT_SUCCESS);
}

llvm::TargetMachine *
createTargetMachine(const char *Argv0) {
    // 收集命令行提供的所有信息，以下是代码生成器的选项、CPU的名称、应该开启(或关闭)的可能特性以及目标的"三元组":
    llvm::Triple Triple = llvm::Triple(!MTriple.empty() ? llvm::Triple::normalize(MTriple) :
            llvm::sys::getDefaultTargetTriple());

    llvm::TargetOptions TargetOptions = llvm::codegen::InitTargetOptionsFromCodeGenFlags(Triple);
    std::string CPUStr = llvm::codegen::getCPUStr();
    std::string FeatureStr = llvm::codegen::getFeaturesStr();
    // 在目标注册表中查找目标。如果发生错误，则显示错误消息并退出。一个可能的错误是，用户指定的不支持的三元组:
    std::string Error;
    const llvm::Target *Target = llvm::TargetRegistry::lookupTarget(llvm::codegen::getMArch(), Triple, Error);
    if (!Target) {
        llvm::WithColor::error(llvm::errs(), Argv0) << Error;
        return nullptr;
    }
    // 在Target类的帮助下，使用用户请求的所有已知选项配置目标机器：
    llvm::TargetMachine *TM = Target->createTargetMachine(Triple.getTriple(), CPUStr, FeatureStr,
                                                          TargetOptions,
                                                          llvm::Optional<llvm::Reloc::Model>(
                                                                  llvm::codegen::getRelocModel()
                                                                  ));
    return TM;
}

bool emit(llvm::StringRef Argv0, llvm::Module *M, llvm::TargetMachine *TM, llvm::StringRef InputFilename)
{
    llvm::CodeGenFileType FileType = llvm::codegen::getFileType();
    std::string OutputFilename;
    if (InputFilename == "-") {
        OutputFilename = "-";
    } else {
        if (InputFilename.endswith(".mod") ||
            InputFilename.endswith(".mod"))
            OutputFilename = InputFilename.drop_back(4).str();
        else
            OutputFilename = InputFilename.str();
        switch (FileType) {
            case llvm::CGFT_AssemblyFile:
                OutputFilename.append(EmitLLVM ? ".ll" : ".s");
                break;
            case llvm::CGFT_ObjectFile:
                OutputFilename.append(".o");
                break;
            case llvm::CGFT_Null:
                OutputFilename.append(".null");
                break;
        }
    }

    // Open the file.
    std::error_code EC;
    llvm::sys::fs::OpenFlags OpenFlags = llvm::sys::fs::OF_None;
    if (FileType == llvm::CGFT_AssemblyFile)
        OpenFlags |= llvm::sys::fs::OF_Text;
    auto Out = std::make_unique<llvm::ToolOutputFile>(OutputFilename, EC, OpenFlags);
    if (EC) {
        llvm::WithColor::error(llvm::errs(), Argv0) << EC.message() << '\n';
        return false;
    }

    llvm::legacy::PassManager PM;
    if (FileType == llvm::CGFT_AssemblyFile && EmitLLVM) {
        PM.add(llvm::createPrintModulePass(Out->os()));
    } else {
        if (TM->addPassesToEmitFile(PM, Out->os(), nullptr, FileType)) {
            llvm::WithColor::error() << "No support for file type\n";
            return false;
        }
    }
    PM.run(*M);
    Out->keep();
    return true;
}

int main(int argc, const char **argv_)
{
    llvm::InitLLVM X(argc, argv_);

    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    llvm::cl::SetVersionPrinter(&printVersion);
    llvm::cl::ParseCommandLineOptions(argc, argv_, Head);

    if (llvm::codegen::getMCPU() == "help" ||
        std::any_of(llvm::codegen::getMAttrs().begin(), llvm::codegen::getMAttrs().end(),
                    [](const std::string &a){
                        return a == "help";
        })) {
        auto Triple = llvm::Triple(LLVM_DEFAULT_TARGET_TRIPLE);
        std::string ErrMsg;
        if (auto target = llvm::TargetRegistry::lookupTarget(Triple.getTriple(), ErrMsg)) {
            llvm::errs() << "Targeting" << target->getName() << ". ";
            // this prints the available CPUs and features of the
            // target to stderr...
            target->createMCSubtargetInfo(Triple.getTriple(),
                                          llvm::codegen::getCPUStr(),
                                          llvm::codegen::getFeaturesStr());
        } else {
            llvm::errs() << ErrMsg << "\n";
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

//    llvm::SmallVector<const char *, 256> argv(argv_ + 1, argv_ + argc);
//
//    llvm::outs() << "Tinylang "
//                 << getTinylangVersion() << "\n";
//
//    for (const char *F : argv) {
//        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
//            FileOrErr = llvm::MemoryBuffer::getFile(F);
//        if (std::error_code BufferError = FileOrErr.getError()) {
//            llvm::errs() << "Error reading " << F << ": "
//                         << BufferError.message() << "\n";
//            continue;
//        }
//
//        // Source Message report, llvm发出错误消息的类
//        llvm::SourceMgr SrcMgr;
//        DiagnosticsEngine Diags(SrcMgr);
//
//        // Tell SrcMgr about this buffer, which is what the
//        // parser will pick up.
//        SrcMgr.AddNewSourceBuffer(std::move(*FileOrErr), llvm::SMLoc());
//
//        auto lexer = Lexer(SrcMgr, Diags);
//        auto sema = Sema(Diags);
//        auto parser = Parser(lexer, sema);
//        parser.parse();
//    }

    llvm::TargetMachine *TM = createTargetMachine(argv_[0]);
    if (!TM)
        exit(EXIT_FAILURE);

    for (const auto  &F : InputFiles) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileOrErr = llvm::MemoryBuffer::getFile(F);
        if (std::error_code BufferError = FileOrErr.getError()) {
            llvm::WithColor::error(llvm::errs(), argv_[0])
                << "Error reading " << F << ": "
                << BufferError.message() << "\n";
        }

        llvm::SourceMgr SrcMgr;
        DiagnosticsEngine Diags(SrcMgr);

        // Tell SrcMgr about this buffer, which is what the parser will pick up.
        SrcMgr.AddNewSourceBuffer(std::move(*FileOrErr), llvm::SMLoc());

        auto lexer = Lexer(SrcMgr, Diags);
        auto sema = Sema(Diags);
        auto parser = Parser(lexer, sema);
        auto *Mod = parser.parse();
        if (Mod && !Diags.nunErrors()) {
            llvm::LLVMContext Ctx;
            if (CodeGenerator *CG = CodeGenerator::create(Ctx, TM)) {
                std::unique_ptr<llvm::Module> M = CG->run(Mod, F);
                if (!emit(argv_[0], M.get(), TM, F)) {
                    llvm::WithColor::error(llvm::errs(), argv_[0]) << "Error writing output\n";
                }
                delete CG;
            }
        }
    }

}
