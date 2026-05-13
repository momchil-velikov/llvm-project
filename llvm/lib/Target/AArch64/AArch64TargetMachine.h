//==-- AArch64TargetMachine.h - Define TargetMachine for AArch64 -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the AArch64 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64TARGETMACHINE_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64TARGETMACHINE_H

#include "AArch64InstrInfo.h"
#include "AArch64Subtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/IR/DataLayout.h"
#include <optional>

namespace llvm {

struct AArch64SubtargetMapKey {
  SmallString<32> CPU;
  SmallString<32> TuneCPU;
  SmallString<128> FS;
  unsigned MinSVEVectorSize = 0;
  unsigned MaxSVEVectorSize = 0;
  bool IsStreaming = false;
  bool IsStreamingCompatible = false;
  bool HasMinSize = false;

  AArch64SubtargetMapKey() = default;

  AArch64SubtargetMapKey(unsigned MinSVEVectorSize, unsigned MaxSVEVectorSize,
                         bool IsStreaming, bool IsStreamingCompatible,
                         bool HasMinSize, StringRef CPU, StringRef TuneCPU,
                         StringRef FS)
      : CPU(CPU), TuneCPU(TuneCPU), FS(FS), MinSVEVectorSize(MinSVEVectorSize),
        MaxSVEVectorSize(MaxSVEVectorSize), IsStreaming(IsStreaming),
        IsStreamingCompatible(IsStreamingCompatible), HasMinSize(HasMinSize) {}

  bool operator==(const AArch64SubtargetMapKey &Other) const {
    return MinSVEVectorSize == Other.MinSVEVectorSize &&
           MaxSVEVectorSize == Other.MaxSVEVectorSize &&
           IsStreaming == Other.IsStreaming &&
           IsStreamingCompatible == Other.IsStreamingCompatible &&
           HasMinSize == Other.HasMinSize && CPU == Other.CPU &&
           TuneCPU == Other.TuneCPU && FS == Other.FS;
  }
};

template <> struct DenseMapInfo<AArch64SubtargetMapKey> {
  static inline AArch64SubtargetMapKey getEmptyKey() {
    AArch64SubtargetMapKey Key;
    Key.MinSVEVectorSize = std::numeric_limits<unsigned>::max();
    return Key;
  }

  static inline AArch64SubtargetMapKey getTombstoneKey() {
    AArch64SubtargetMapKey Key;
    Key.MinSVEVectorSize = std::numeric_limits<unsigned>::max() - 1;
    return Key;
  }

  static unsigned getHashValue(const AArch64SubtargetMapKey &Key) {
    return static_cast<unsigned>(hash_combine(
        Key.MinSVEVectorSize, Key.MaxSVEVectorSize, Key.IsStreaming,
        Key.IsStreamingCompatible, Key.HasMinSize, StringRef(Key.CPU),
        StringRef(Key.TuneCPU), StringRef(Key.FS)));
  }

  static bool isEqual(const AArch64SubtargetMapKey &LHS,
                      const AArch64SubtargetMapKey &RHS) {
    return LHS == RHS;
  }
};

class AArch64TargetMachine : public CodeGenTargetMachineImpl {
protected:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable DenseMap<AArch64SubtargetMapKey, std::unique_ptr<AArch64Subtarget>>
      SubtargetMap;

  /// Reset internal state.
  void reset() override;

public:
  AArch64TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT, bool IsLittleEndian);

  ~AArch64TargetMachine() override;
  const AArch64Subtarget *getSubtargetImpl(const Function &F) const override;
  // DO NOT IMPLEMENT: There is no such thing as a valid default subtarget,
  // subtargets are per-function entities based on the target-specific
  // attributes of each function.
  const AArch64Subtarget *getSubtargetImpl() const = delete;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  void registerPassBuilderCallbacks(PassBuilder &PB) override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetLoweringObjectFile* getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  yaml::MachineFunctionInfo *createDefaultFuncInfoYAML() const override;
  yaml::MachineFunctionInfo *
  convertFuncInfoToYAML(const MachineFunction &MF) const override;
  bool parseMachineFunctionInfo(const yaml::MachineFunctionInfo &,
                                PerFunctionMIParsingState &PFS,
                                SMDiagnostic &Error,
                                SMRange &SourceRange) const override;

  /// Returns true if a cast between SrcAS and DestAS is a noop.
  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    return getPointerSize(SrcAS) == getPointerSize(DestAS);
  }
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;

  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override;

  size_t clearLinkerOptimizationHints(
      const SmallPtrSetImpl<MachineInstr *> &MIs) const override;

  /// Returns the optimisation level that enables GlobalISel.
  unsigned getEnableGlobalISelAtO() const;

private:
  bool isLittle;
};

// AArch64 little endian target machine.
//
class AArch64leTargetMachine : public AArch64TargetMachine {
  virtual void anchor();

public:
  AArch64leTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                         StringRef FS, const TargetOptions &Options,
                         std::optional<Reloc::Model> RM,
                         std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                         bool JIT);
};

// AArch64 big endian target machine.
//
class AArch64beTargetMachine : public AArch64TargetMachine {
  virtual void anchor();

public:
  AArch64beTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                         StringRef FS, const TargetOptions &Options,
                         std::optional<Reloc::Model> RM,
                         std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                         bool JIT);
};

} // end namespace llvm

#endif
