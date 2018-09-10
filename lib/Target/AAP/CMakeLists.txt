set(LLVM_TARGET_DEFINITIONS AAP.td)

tablegen(LLVM AAPGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM AAPGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM AAPGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM AAPGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM AAPGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM AAPGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM AAPGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM AAPGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM AAPGenCallingConv.inc -gen-callingconv)
tablegen(LLVM AAPGenDAGISel.inc -gen-dag-isel)

add_public_tablegen_target(AAPCommonTableGen)

add_llvm_target(AAPCodeGen
  AAPAsmPrinter.cpp
  AAPFrameLowering.cpp
  AAPInstrInfo.cpp
  AAPISelDAGToDAG.cpp
  AAPISelLowering.cpp
  AAPMachineFunctionInfo.cpp
  AAPMCInstLower.cpp
  AAPRegisterInfo.cpp
  AAPShortInstrPeephole.cpp
  AAPSubtarget.cpp
  AAPTargetMachine.cpp
)

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
