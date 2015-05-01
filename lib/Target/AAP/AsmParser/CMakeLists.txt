add_llvm_library(LLVMAAPAsmParser
  AAPAsmParser.cpp
  )

add_dependencies(LLVMAAPAsmParser AAPCommonTableGen)
