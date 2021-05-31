/// \file RemoveExceptionalCalls.cpp
/// \brief

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "revng/FunctionIsolation/RemoveTutteCose.h"
#include "revng/Support/FunctionTags.h"
#include "revng/Support/IRHelpers.h"

using namespace llvm;

char RemoveTutteCose::ID = 0;

using Register = RegisterPass<RemoveTutteCose>;
static Register
  X("remove-tutte-cose", "Remove Tutte Cose");

bool RemoveTutteCose::runOnModule(llvm::Module &M) {
  LLVMContext &C = M.getContext();
  std::set<Function *> ToErase;

  M.getFunction("root")->eraseFromParent();
  for (Function &F : FunctionTags::Lifted.functions(&M)) {
    if (F.getName() != "bb.looks_like_integer")
      F.setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
  }

  return true;
}
