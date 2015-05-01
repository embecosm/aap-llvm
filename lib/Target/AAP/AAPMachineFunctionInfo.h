//===- AAPMachineFuctionInfo.h - AAP machine func info -----------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares AAP-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef AAPMACHINEFUNCTIONINFO_H
#define AAPMACHINEFUNCTIONINFO_H
#include "AAPRegisterInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

namespace llvm {

/// AAPMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private AAP target-specific information for each MachineFunction.
class AAPMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

  MachineFunction &MF;

  /// CalleeSavedFrameSize - Size of the callee-saved register portion of the
  /// stack frame in bytes
  unsigned CalleeSavedFrameSize;

  /// SRetReturnReg - AAP ABI require that sret lowering includes
  /// returning the value of the returned struct in a register. This field
  /// holds the virtual register into which the sret argument is passed.
  unsigned SRetReturnReg;

  /// GlobalBaseReg - keeps track of the virtual register initialized for
  /// use as the global base register. This is used for PIC in some PIC
  /// relocation models.
  unsigned GlobalBaseReg;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

public:
  AAPMachineFunctionInfo(MachineFunction &MF)
      : MF(MF), CalleeSavedFrameSize(0), SRetReturnReg(0), GlobalBaseReg(0),
        VarArgsFrameIndex(0) {}

  unsigned getCalleeSavedFrameSize() { return CalleeSavedFrameSize; }
  void setCalleeSavedFrameSize(unsigned bytes) { CalleeSavedFrameSize = bytes; }

  unsigned getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

  unsigned getGlobalBaseReg();

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
};

} // End llvm namespace

#endif
