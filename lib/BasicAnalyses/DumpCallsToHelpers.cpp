/// \file EmptyNewPC.cpp
/// \brief A simple pass to given an empty body to the `newpc` function so that
///        it can be optimized away.

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include <string>
#include <fstream>
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#include "revng/BasicAnalyses/DumpCallsToHelpers.h"
#include "revng/Support/IRHelpers.h"
#include "revng/Support/FunctionTags.h"

using namespace llvm;

char DumpCallsToHelpers::ID = 0;
using Register = RegisterPass<DumpCallsToHelpers>;
static Register X("dump-calls-to-helpers", "Output which functions call helpers", false, true);
static cl::opt<std::string> OutputFilename("dump-calls-to-helpers-output", cl::desc("Specify output filename for dump-calls-to-helpers"), cl::value_desc("filename"));

bool DumpCallsToHelpers::runOnModule(llvm::Module &M) {
  LLVMContext &Context = getContext(&M);

  std::ofstream Output(OutputFilename.c_str());

  for(auto &F: M.functions()) {
    for (BasicBlock &BB : F) {
      for (Instruction &II : BB) {
        Instruction *I = &II;
        if (CallInst *CI = dyn_cast<CallInst>(I)) {
          auto *CalledFunction = CI->getCalledFunction();
          if (CalledFunction != nullptr) {
            auto Tags = FunctionTags::TagsSet::from(CalledFunction);

            if (Output.good()) {
              Output << F.getName().str();
              Output << ",";
              Output << CalledFunction->getName().str();
              Output << ",";
              if (Tags.contains(FunctionTags::Lifted)) {
                Output << "lifted";
              }
              else {
                Output << "not-lifted";
              }
              Output << "\n";
            }
          }
        }
      }
    }
  }
  return false;
}
