set(LLVM_TARGET_DEFINITIONS AAP.td)

tablegen(LLVM AAPGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM AAPGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM AAPGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM AAPGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM AAPGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM AAPGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM AAPGenDisassemblerTables.inc -gen-disassembler)

add_llvm_target(AAPCodeGen
  AAPTargetMachine.cpp
)

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
