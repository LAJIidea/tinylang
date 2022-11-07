add_tinylang_executable(MarcoTest MarcoTest.cpp)
message("=============" ${LLVM_COMMON_LIBS})
target_link_libraries(MarcoTest INTERFACE ${LLVM_COMMON_LIBS})