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

#include <chrono>
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
    bool TextFlag = Address & 0x8000000;
    StringRef BytesStr;
    Section.getContents(BytesStr);
    std::string Type = (std::string(Text ? "TEXT " : "") +
                        (Data ? "DATA " : "") + (BSS ? "BSS" : ""));
    // FIXME: We should really load based on LOAD segment flag
    if (Text || Data) {
      outs() << format("%3d %-13s %08" PRIx64 " %016" PRIx64 " %s\n", i,
                       Name.str().c_str(), Size, Address, Type.c_str());
      if (TextFlag) {
        Address = Address & 0xffffff;
        outs() << format("Writing %s to %06" PRIx64 "\n", Name.str().c_str(), Address);
        Sim.WriteCodeSection(BytesStr, Address);
      } else {
        Address = Address & 0xffff;
        outs() << format("Writing %s to %04" PRIx64 "\n", Name.str().c_str(), Address);
        Sim.WriteDataSection(BytesStr, Address);
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

static cl::opt<bool>
DebugTrace("debug-trace", cl::desc("Enable debug tracing"));

static cl::opt<bool>
DebugTrace2("d", cl::desc("Enable debug tracing"), cl::Hidden);

static cl::opt<double>
Timeout("timeout", cl::desc("Timeout in seconds"), cl::value_desc("duration"), cl::init(0.0));

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
  Sim.setTracing(DebugTrace || DebugTrace2);

  // Set up timeout if specified
  std::chrono::time_point <std::chrono::system_clock, std::chrono::duration<double> > timeoutEnd =
      std::chrono::system_clock::now() + std::chrono::duration<double>(Timeout);
  bool timeoutActive = Timeout > 0.0;

  ToolName = argv[0];

  // Load Binary
  LoadBinary(Sim, InputFilename);
  unsigned stepCount = 0;
  SimStatus status = SimStatus::SIM_OK;
  while (status == SimStatus::SIM_OK) {
    status = Sim.step();
    if (timeoutActive && stepCount++ == 10000) {
      stepCount = 0;
      if (std::chrono::system_clock::now() > timeoutEnd) {
        status = SimStatus::SIM_TIMEOUT;
        break;
      }
    }
  }

  // Deal with the final simulator status
  switch (status) {
    default: break;
    case SimStatus::SIM_INVALID_INSN:
      outs() << " *** Attempted to decode invalid instruction ***\n";
      break;
    case SimStatus::SIM_BREAKPOINT:
      outs() << " *** Breakpoint hit ***\n";
      break;
    case SimStatus::SIM_TRAP:
      outs() << " *** Simulator trap ***\n";
      break;
    case SimStatus::SIM_EXCEPT_MEM:
      outs() << " *** Invalid memory trap ***\n";
      return 1;
    case SimStatus::SIM_EXCEPT_REG:
      outs() << " *** Invalid register trap ***\n";
      return 1;
    case SimStatus::SIM_TIMEOUT:
      outs() << " *** Simulator timeout ***\n";
      return 1;
    case SimStatus::SIM_QUIT:
      outs() << " *** EXIT CODE " << Sim.getState().getExitCode() << " ***\n";
      return Sim.getState().getExitCode();
  }

  return 0;
}
