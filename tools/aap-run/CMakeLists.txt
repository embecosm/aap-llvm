set(LLVM_LINK_COMPONENTS
  AllTargetsAsmPrinters
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  CodeGen
  DebugInfoDWARF
  MC
  MCDisassembler
  Object
  Support
  AAPSim
  )

# FIXME: Eventually move headers to include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/Target/AAPSimulator/
                    ${CMAKE_CURRENT_BINARY_DIR}/../../lib/Target/AAPSimulator/ )

add_llvm_tool(aap-run
  AAPSimTest.cpp
  )
