#ifndef LLVM_ANALYSIS_MARKER_H
#define LLVM_ANALYSIS_MARKER_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/ExtraPassManager.h"

namespace llvm {
/// A function level marker analysis to determine if inliner should run again.
struct ShouldRunExtraInliner
    : public ShouldRunExtraPasses<ShouldRunExtraInliner>,
      public AnalysisInfoMixin<ShouldRunExtraInliner> {
  LLVM_ABI static AnalysisKey Key;
};

/// A module level marker analysis to determine if inliner should run again.
struct ShouldRunExtraInlinerModule
    : public ShouldRunExtraPasses<ShouldRunExtraInlinerModule>,
      public AnalysisInfoMixin<ShouldRunExtraInlinerModule> {
  LLVM_ABI static AnalysisKey Key;
};

/// A pass to promote the ShouldRunExtraInliner analysis from function level to
/// module level.
struct PromoteExtraInlinerMarker
    : public FunctionToModuleMarkerBridge<ShouldRunExtraInlinerModule,
                                          ShouldRunExtraInliner> {
  LLVM_ABI static AnalysisKey Key;
};

} // namespace llvm

#endif // LLVM_ANALYSIS_MARKER_H
