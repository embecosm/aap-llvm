##===- lib/Target/AAP/Makefile ---------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMAAPCodeGen
TARGET = AAP

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = AAPGenRegisterInfo.inc \
		AAPGenInstrInfo.inc \
		AAPGenAsmWriter.inc \
		AAPGenAsmMatcher.inc \
		AAPGenDAGISel.inc \
		AAPGenDisassemblerTables.inc \
		AAPGenSubtargetInfo.inc
		AAPGenCallingConv.inc \
		AAPGenMCCodeEmitter.inc

DIRS = AsmParser Disassembler InstPrinter MCTargetDesc TargetInfo

include $(LEVEL)/Makefile.common
