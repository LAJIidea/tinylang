set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Analysis
        CodeGen
        Core
        IPO
        AggressiveInstCombine
        InstCombine
        Instrumentation
        MC
        MCParser
        ObjCARCOpts
        Option
        ScalarOpts
        Support
        TransformUtils
        Vectorize
        )

add_tinylang_tool(tinylang
        Driver.cpp
        )


target_link_libraries(tinylang
        PRIVATE
        tinylangBasic
        tinylangLexer
        tinylangParser
        tinylangSema
        tinylangCodeGen
        )