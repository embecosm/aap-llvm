//===-- AAPFixupKinds.h - AAP Specific Fixup Entries ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_AAP_AAPFIXUPKINDS_H
#define LLVM_AAP_AAPFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace AAP {
// Although most of the current fixup types reflect a unique relocation
// one can have multiple fixup types for a given relocation and thus need
// to be uniquely named.
//
// This table *must* be in the save order of
// MCFixupKindInfo Infos[AAP::NumTargetFixupKinds]
// in AAPAsmBackend.cpp.
//
enum Fixups {
  // Results in R_AAP_NONE
  fixup_AAP_NONE = FirstTargetFixupKind,

  // Fixup for resolving branches to other basic blocks
  fixup_AAP_BR16,
  fixup_AAP_BR32,

  // Fixup for resolving conditional branches to other basic blocks
  fixup_AAP_BRCC16,
  fixup_AAP_BRCC32,

  // Branch and links to other basic blocks
  fixup_AAP_BAL16,
  fixup_AAP_BAL32,

  // Fixup for absolute addresses
  fixup_AAP_ABS3_SHORT,
  fixup_AAP_ABS6_SHORT,

  fixup_AAP_ABS6,
  fixup_AAP_ABS9,
  fixup_AAP_ABS10,
  fixup_AAP_ABS12,
  fixup_AAP_ABS16,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
} // end namespace AAP
} // end namespace llvm

#endif
