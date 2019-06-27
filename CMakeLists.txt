set(LLVM_LINK_COMPONENTS support)
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

add_clang_executable(self-inc-first
                     src/IncludeFinderAction.cpp
                     src/path_utils.cpp)
add_definitions("-Wall -g")

target_link_libraries(self-inc-first 
 PRIVATE
 clangTooling 
 clangBasic 
 clangASTMatchers
 )

