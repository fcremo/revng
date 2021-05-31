/// \file RemoveCallsToAllocaLeakHelpers.cpp
/// \brief

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

#include "revng/FunctionIsolation/ReplaceInitSPReg.h"
#include "revng/Support/FunctionTags.h"
#include "revng/Support/IRHelpers.h"

using namespace llvm;

char ReplaceInitSPReg::ID = 0;

using Register = RegisterPass<ReplaceInitSPReg>;
static Register
  X("replace-init-spreg", "Replace calls to init_spreg");

bool ReplaceInitSPReg::runOnModule(llvm::Module &M) {
  for (auto User : M.getFunction("init_spreg")->users()) {
    auto *CallInstUser = cast<CallInst>(User);
    auto *AllocaSize = cast<ConstantInt>(CallInstUser->getArgOperand(0));

    IRBuilder<> AllocaBuilder(CallInstUser);
    auto *AllocaInstr = AllocaBuilder.CreateAlloca(AllocaBuilder.getInt8Ty(), AllocaSize, "TopOfTheStack");
    CallInstUser->replaceAllUsesWith(AllocaInstr);
    CallInstUser->eraseFromParent();
  }
  M.getFunction("init_spreg")->eraseFromParent();

  return true;
}
