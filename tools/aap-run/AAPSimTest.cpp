//===-- AAPSimTest.cpp - AAP Simulator Test  --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Test program for using AAP Simulation Library
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "AAPSimulator.h"
#include "AAPSimState.h"

using namespace llvm;
using namespace object;
using namespace AAPSim;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::Required, cl::desc("<input object>"));

static std::string ToolName;

static void report_error(StringRef File, std::error_code EC) {
  assert(EC);
  errs() << ToolName << ": '" << File << "': " << EC.message() << ".\n";
  exit(1);
}

static void LoadObject(AAPSimulator &Sim, ObjectFile *o) {
  unsigned i = 0;
  for (const SectionRef &Section : o->sections()) {
    StringRef Name;
    Section.getName(Name);
    uint64_t Address = Section.getAddress();
    uint64_t Size = Section.getSize();
    bool Text = Section.isText();
    bool Data = Section.isData();
    bool BSS = Section.isBSS();
    StringRef BytesStr;
    Section.getContents(BytesStr);
    std::string Type = (std::string(Text ? "TEXT " : "") +
                        (Data ? "DATA " : "") + (BSS ? "BSS" : ""));
    // FIXME: We should really load based on LOAD segment flag
    if (Text || Data) {
      outs() << format("%3d %-13s %08" PRIx64 " %016" PRIx64 " %s\n", i,
                       Name.str().c_str(), Size, Address, Type.c_str());
      // FIXME: Flag conversion
      if (Text) {
        Address = Address & 0xffffff;
        outs() << format("Writing %s to %08" PRIx64 "\n", Name.str().c_str(), Address);
        Sim.WriteCodeSection(BytesStr, Address);
      }
    }
    ++i;
  }
  // Set PC
  Sim.setPC(0x0);
}

// Load an object
static void LoadBinary(AAPSimulator &Sim, std::string filename) {
  // Attempt to open the binary.
  ErrorOr<OwningBinary<Binary>> BinaryOrErr = createBinary(filename);
  if (std::error_code EC = BinaryOrErr.getError())
    report_error(filename, EC);
  Binary &Binary = *BinaryOrErr.get().getBinary();
  if (ObjectFile *o = dyn_cast<ObjectFile>(&Binary))
    LoadObject(Sim, o);
  else
    report_error(filename, object_error::invalid_file_type);
}

int main(int argc, char **argv) {
  // Init LLVM, call llvm_shutdown() on exit, parse args, etc.
  PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj Y;

  // Initialize targets and assembly printers/parsers.
  // (FIXME: Move this into AAPSimulator?)
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllDisassemblers();

  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);
  cl::ParseCommandLineOptions(argc, argv, "AAP Simulator Test\n");

  // Set up Simulator
  AAPSimulator Sim;

  ToolName = argv[0];

  // Load Binary
  LoadBinary(Sim, InputFilename);
  SimStatus status = SimStatus::SIM_OK;
  while (status == SimStatus::SIM_OK) {
    status = Sim.step();
  }

  return 0;
}