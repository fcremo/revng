#pragma once

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include <string>

#include "llvm/Pass.h"

#include "revng/BasicAnalyses/GeneratedCodeBasicInfo.h"
#include "revng/FunctionCallIdentification/FunctionCallIdentification.h"
#include "revng/Model/LoadModelPass.h"
#include "revng/StackAnalysis/FunctionsSummary.h"

namespace StackAnalysis {

extern const std::set<llvm::GlobalVariable *> EmptyCSVSet;

template<bool AnalyzeABI>
class StackAnalysis : public llvm::ModulePass {
  friend class FunctionBoundariesDetectionPass;

public:
  static char ID;

public:
  StackAnalysis() : llvm::ModulePass(ID) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<GeneratedCodeBasicInfo>();
    AU.addRequired<LoadModelPass>();
  }

  bool runOnModule(llvm::Module &M) override;

  const std::set<llvm::GlobalVariable *> &
  getClobbered(llvm::BasicBlock *Function) const {
    auto It = GrandResult.Functions.find(Function);
    if (It == GrandResult.Functions.end())
      return EmptyCSVSet;
    else
      return It->second.ClobberedRegisters;
  }

  void serialize(std::ostream &Output) { Output << TextRepresentation; }

  void serializeMetadata(llvm::Function &F);

public:
  FunctionsSummary GrandResult;
  std::string TextRepresentation;
};

template<>
char StackAnalysis<true>::ID;

template<>
char StackAnalysis<false>::ID;

extern template void StackAnalysis<true>::serializeMetadata(llvm::Function &F);
extern template void StackAnalysis<false>::serializeMetadata(llvm::Function &F);

} // namespace StackAnalysis
