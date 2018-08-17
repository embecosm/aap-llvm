set(LLVM_TARGET_DEFINITIONS AAP.td)

add_llvm_target(AAPCodeGen
  AAPTargetMachine.cpp
)

add_subdirectory(TargetInfo)
