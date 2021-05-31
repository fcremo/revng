#pragma once

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include "llvm/Pass.h"

class ReplaceInitSPReg : public llvm::ModulePass {
public:
  static char ID;

public:
  ReplaceInitSPReg() : llvm::ModulePass(ID) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(llvm::Module &F) override;
};
