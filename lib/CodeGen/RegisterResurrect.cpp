//===------- RegisterResurrect.cpp - Call site register resurrection ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the RegisterResurrect class, which implements an
// optimization which resurrects a register by removing redundant Defs of a
// register at a call site, where it can be determined that the register is
// not clobbered by the call.
//
// This pass executes in a bottom-up traversal of the call graph, which allows
// the caller to determine the exact physical registers used in the callee.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/BitVector.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "register-resurrect"

namespace {

class RegisterResurrect : public CallGraphSCCPass {
public:
  static char ID;
  RegisterResurrect() : CallGraphSCCPass(ID) {
    initializeRegisterResurrectPass(*PassRegistry::getPassRegistry());
  }

  bool runOnSCC(CallGraphSCC &SCC) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<MachineModuleInfo>();
    AU.addPreserved<MachineModuleInfo>();

    // We need MachineFunctionAnalysis to have run at least once, otherwise
    // MachineModuleInfo will not have a Function->MachineFunction mapping.
    // This should only be a problem if this is the first pass run which
    // requires a MachineFunction. In this case the analysis may need to be
    // triggered manually.
    CallGraphSCCPass::getAnalysisUsage(AU);
  }

private:
  MachineModuleInfo *MMI;

  bool runOnMachineFunction(MachineFunction &MF);
  bool runOnMachineBasicBlock(MachineFunction &MF, MachineBasicBlock &MBB);

  const uint32_t *getClobbersForFunction(const TargetRegisterInfo &TRI,
                                         const Function &MF);

  static void printRegMask(raw_ostream &OS, const TargetRegisterInfo &TRI,
                           const uint32_t *Mask);
};

} // end of anonymous namespace


bool RegisterResurrect::runOnSCC(CallGraphSCC &SCC)
{
  MMI = &getAnalysis<MachineModuleInfo>();

  // Multiple nodes in an SCC signify indirect recursion. Ignore this case
  // for now.
  if (SCC.size() > 1)
    return false;

  const CallGraphNode* CGNode = *SCC.begin();
  const Function* Fn = CGNode->getFunction();

  // Ignore null function/external nodes.
  if (!Fn)
    return false;

  // If the function calls itselfs we have direct recursion, ignore this case
  for (unsigned i = 0; i != CGNode->size(); ++i) {
    if ((*CGNode)[i] == CGNode)
      return false;
  }

  // For previously encountered functions, there should be a corresponding
  // MachineFunction. This is not true of functions such as intrinsics, so
  // ignore these cases.
  MachineFunction *MF = MMI->getMachineFunction(Fn);
  if (!MF)
    return false;

  return runOnMachineFunction(*MF);
}

bool RegisterResurrect::runOnMachineFunction(MachineFunction &MF)
{
  bool Modified = false;
  for (auto &MBB : MF) {
    if (runOnMachineBasicBlock(MF, MBB))
      Modified = true;
  }
  return Modified;
}

bool RegisterResurrect::runOnMachineBasicBlock(MachineFunction &MF,
                                               MachineBasicBlock &MBB)
{
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  unsigned NumRegs = TRI.getNumRegs();

  bool Modified = false;
  for (auto &MI : MBB) {
    if (!MI.isCall())
      continue;

    // search the operands for call target
    const Function* Fn = nullptr;
    for (unsigned i = 0; i < MI.getNumOperands(); ++i) {
      if (!MI.getOperand(i).isGlobal())
        continue;

      // FIXME: we assume any resolvable global value is the called function
      if ((Fn = dyn_cast<Function>(MI.getOperand(i).getGlobal())))
        break;
    }

    // Skip an unresolvable call
    if (!Fn)
      continue;

    // Add registers to the preserved mask which are not clobbered in the
    // called function
    DEBUG(dbgs() << "\nResurrecting:\n"; MI.dump());
    const uint32_t *Clobbers = getClobbersForFunction(TRI, *Fn);
    if (!Clobbers) {
      DEBUG(dbgs() << "No clobbers");
      continue;
    }
    DEBUG(dbgs() << "Clobbers for " << Fn->getName() << ":\n  ";
          printRegMask(dbgs(), TRI, Clobbers));

    for (unsigned Op = 0; Op < MI.getNumOperands(); ++Op) {
      if (!MI.getOperand(Op).isRegMask())
        continue;

      // FIXME: Massive memory leak
      const uint32_t *PreservedMask = MI.getOperand(Op).getRegMask();
      uint32_t *NewPreservedMask = new uint32_t[(NumRegs + 31) / 32];

      memcpy(NewPreservedMask, PreservedMask, NumRegs / 8);
      for (unsigned i = 0; i < (NumRegs + 31) / 32; ++i) {
        NewPreservedMask[i] |= ~Clobbers[i];
      }

      DEBUG(dbgs() << "\nUpdating preserved mask";
            dbgs() << "\n  Old: "; printRegMask(dbgs(), TRI, PreservedMask);
            dbgs() << "\n  New: "; printRegMask(dbgs(), TRI, NewPreservedMask));

      MI.getOperand(Op).ChangeToRegMask(NewPreservedMask);
      Modified = true;
    }
  }
  return Modified;
}

/// getClobbersForFunction - Retrieve all of the physical registers which
/// are defined by instruction in the provided MachineFunction. At this
/// point, all virtual registers in the function should have been resolved to
/// physical ones.
const uint32_t *RegisterResurrect::getClobbersForFunction(
    const TargetRegisterInfo &TRI, const Function &F)
{
  if (const uint32_t *Clobbers = MMI->getClobbersForFunction(F))
    return Clobbers;

  MachineFunction *MF = MMI->getMachineFunction(&F);
  // Function may be external, or empty (if it has been inlined)
  if (!MF || MF->size() == 0)
    return nullptr;

  // Not cached, calculate clobbers
  uint32_t NumRegs = TRI.getNumRegs();
  uint32_t *Clobbers = new uint32_t[(NumRegs + 31) / 32];
  for (unsigned i = 0; i < (NumRegs + 31) / 32; ++i)
    Clobbers[i] = 0;

  for (auto &MBB : *MF) {
    for (auto &MI : MBB) {
      for (unsigned i = 0; i < MI.getNumOperands(); ++i) {
        const MachineOperand &MO = MI.getOperand(i);
        if (!MO.isReg() && !MO.isRegMask())
          continue;

        if (MO.isReg()) {
          unsigned Reg = MI.getOperand(i).getReg();
          assert (Reg <= NumRegs &&
                  "Virtual register encountered calculating function clobbers");

          // A register is only clobbered if it is a def for an operand
          // Sub/super registers?
          if (MO.isDef())
            Clobbers[Reg / 32] |= (1 << (Reg % 32));
        }
        else if (MO.isRegMask()) {
          // Clobber anything not preserved in the mask
          const uint32_t *PreservedMask = MO.getRegMask();
          for (unsigned j = 0; j < (NumRegs + 31) / 32; ++j)
            Clobbers[j] |= ~PreservedMask[j];
        }
      }
    }
  }

  // MMI takes ownership of the clobber mask
  MMI->setClobbersForFunction(F, Clobbers);
  return Clobbers;
}

void RegisterResurrect::printRegMask(raw_ostream &OS,
                                     const TargetRegisterInfo &TRI,
                                     const uint32_t *Mask)
{
  unsigned NumRegs = TRI.getNumRegs();

  OS << "<regmask";
  for (unsigned i = 0; i < NumRegs; ++i) {
    unsigned MaskWord = i / 32;
    unsigned MaskBit = i % 32;
    if (Mask[MaskWord] & (1 << MaskBit))
      OS << " " << PrintReg(i, &TRI);
  }
  OS << ">";
}

INITIALIZE_PASS_BEGIN(RegisterResurrect,
                      "register-resurrect",
                      "Register Resurrection",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(MachineModuleInfo)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(RegisterResurrect,
                    "register-resurrect",
                    "Register Resurrection",
                    false, false)

char RegisterResurrect::ID = 0;
char &llvm::RegisterResurrectID = RegisterResurrect::ID;

CallGraphSCCPass *llvm::createRegisterResurrectPass()
{
  return new RegisterResurrect();
}
