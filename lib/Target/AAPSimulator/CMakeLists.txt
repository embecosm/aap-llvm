# FIXME: Need to include AAP headers because not in lib/Target/AAP
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../AAP
  ${CMAKE_CURRENT_BINARY_DIR}/../AAP
)

add_llvm_library(LLVMAAPSim
  AAPSimState.cpp
  AAPSimulator.cpp
)
