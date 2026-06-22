#include "llvm/Analysis/Utils/Marker.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

using namespace llvm;

AnalysisKey ShouldRunExtraInliner::Key;

AnalysisKey ShouldRunExtraInlinerModule::Key;

AnalysisKey PromoteExtraInlinerMarker::Key;
