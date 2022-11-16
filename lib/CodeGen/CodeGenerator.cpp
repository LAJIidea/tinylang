//
// Created by BY210033 on 2022/11/15.
//
#include "tinylang/CodeGen/CodeGenerator.h"
#include "tinylang/CodeGen/CGModule.h"

using namespace tinylang;
using namespace llvm;

// 为代码生成器引入工厂方式
CodeGenerator *CodeGenerator::create(LLVMContext &Ctx, TargetMachine *TM) {
    return new CodeGenerator(Ctx, TM);
}

/**
 * CodeGenerator类初始化LLVM IR模块，并调用该模块的代码生成。最重要的是，这个类必须知道我们为哪个目标
 * 体系结构生成代码，这个信息在llvm::TargetMachine类中传递，在驱动程序中设置
 * @param CM
 * @param FileName
 * @return
 */
std::unique_ptr<llvm::Module> CodeGenerator::run(ModuleDeclaration *CM, std::string FileName) {
    std::unique_ptr<Module> M = std::make_unique<Module>(FileName, Ctx);
    M->setTargetTriple(TM->getTargetTriple().getTriple());
    M->setDataLayout(TM->createDataLayout());
    CGModule CGM(M.get());
    CGM.run(CM);
    return M;
}
