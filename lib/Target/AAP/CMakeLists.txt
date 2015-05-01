set(LLVM_TARGET_DEFINITIONS AAP.td)

tablegen(LLVM AAPGenRegisterInfo.inc       -gen-register-info)
tablegen(LLVM AAPGenInstrInfo.inc          -gen-instr-info)
tablegen(LLVM AAPGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM AAPGenMCCodeEmitter.inc      -gen-emitter)
tablegen(LLVM AAPGenAsmWriter.inc          -gen-asm-writer)
tablegen(LLVM AAPGenAsmMatcher.inc         -gen-asm-matcher)
tablegen(LLVM AAPGenDAGISel.inc            -gen-dag-isel)
tablegen(LLVM AAPGenSubtargetInfo.inc      -gen-subtarget)
tablegen(LLVM AAPGenCallingConv.inc        -gen-callingconv)
add_public_tablegen_target(AAPCommonTableGen)

add_llvm_target(AAPCodeGen
  AAPFrameLowering.cpp
  AAPInstrInfo.cpp
  AAPISelDAGToDAG.cpp
  AAPISelLowering.cpp
  AAPMachineFunctionInfo.cpp
  AAPRegisterInfo.cpp
  AAPSelectionDAGInfo.cpp
  AAPSubtarget.cpp
  AAPTargetMachine.cpp
  AAPAsmPrinter.cpp
  AAPMCInstLower.cpp
  )

add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(AsmParser)
