//===-- MachineFunctionAnalysis.cpp ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the definitions of the MachineFunctionAnalysis members.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/GCMetadata.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionInitializer.h"
#include "llvm/Pass.h"
using namespace llvm;

char MachineFunctionAnalysis::ID = 0;

MachineFunctionAnalysis::MachineFunctionAnalysis()
    : FunctionPass(ID), TM(nullptr), MF(nullptr), MFInitializer(nullptr) {
  initializeMachineModuleInfoPass(*PassRegistry::getPassRegistry());
}

MachineFunctionAnalysis::~MachineFunctionAnalysis() {
  releaseMemory();
  assert(!MF && "MachineFunctionAnalysis left initialized!");
}

void MachineFunctionAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineModuleInfo>();
}

bool MachineFunctionAnalysis::doInitialization(Module &M) {
  MachineModuleInfo *MMI = getAnalysisIfAvailable<MachineModuleInfo>();
  assert(MMI && "MMI not around yet??");

  TM = MMI->getTargetMachine();
  MFInitializer = MMI->getMachineFunctionInitializer();
  MMI->setModule(&M);
  NextFnNum = 0;
  return false;
}


bool MachineFunctionAnalysis::runOnFunction(Function &F) {
  assert(TM && "No LLVMTargetMachine");
  assert(!MF && "MachineFunctionAnalysis already initialized!");

  MachineModuleInfo &MMI = getAnalysis<MachineModuleInfo>();
  MF = MMI.getMachineFunction(&F);
  if (!MF) {
    MF = new MachineFunction(&F, *TM, NextFnNum++, MMI);
    MMI.addMachineFunction(&F, MF);
  }
  if (MFInitializer)
    MFInitializer->initializeMachineFunction(*MF);
  return false;
}

void MachineFunctionAnalysis::releaseMemory() {
  // MF is owned by MachineModuleInfo, so do not delete here
  MF = nullptr;
}

INITIALIZE_PASS(MachineFunctionAnalysis, "machine-function-analysis",
                "Machine Function Analysis", false, false)
