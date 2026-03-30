#include "Utils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h" // For DemoteRegToStack and DemotePHIToStack
#include <sstream>

using namespace llvm;

std::string readAnnotate(Function *f) {
  std::string annotation = "";

  GlobalVariable *glob =
      f->getParent()->getGlobalVariable("llvm.global.annotations");

  if (!glob)
    return annotation;

  if (ConstantArray *ca = dyn_cast<ConstantArray>(glob->getInitializer())) {
    for (unsigned i = 0; i < ca->getNumOperands(); ++i) {
      if (ConstantStruct *structAn =
              dyn_cast<ConstantStruct>(ca->getOperand(i))) {

        Value *fnOperand = structAn->getOperand(0);
        Function *annotatedFn = dyn_cast<Function>(fnOperand);
        if (!annotatedFn) {
          if (auto *expr = dyn_cast<ConstantExpr>(fnOperand))
            if (expr->getOpcode() == Instruction::BitCast)
              annotatedFn = dyn_cast<Function>(expr->getOperand(0));
        }
        if (annotatedFn != f)
          continue;

        Value *noteOperand = structAn->getOperand(1);
        GlobalVariable *annoteStr = dyn_cast<GlobalVariable>(noteOperand);
        if (!annoteStr) {
          if (auto *expr = dyn_cast<ConstantExpr>(noteOperand))
            if (expr->getOpcode() == Instruction::GetElementPtr)
              annoteStr = dyn_cast<GlobalVariable>(expr->getOperand(0));
        }
        if (annoteStr && annoteStr->hasInitializer()) {
          if (ConstantDataSequential *data =
                  dyn_cast<ConstantDataSequential>(
                      annoteStr->getInitializer())) {
            if (data->isString()) {
              annotation += data->getAsString().lower() + " ";
            }
          }
        }
      }
    }
  }
  return annotation;
}

bool toObfuscate(bool flag, Function *f, std::string attribute) {
  std::string attr = attribute;
  std::string attrNo = "no" + attr;

  // Check if declaration
  if (f->isDeclaration()) {
    return false;
  }

  // Check external linkage
  if (f->hasAvailableExternallyLinkage() != 0) {
    return false;
  }

  // We have to check the nofla flag first
  // Because .find("fla") is true for a string like "fla" or
  // "nofla"
  if (readAnnotate(f).find(attrNo) != std::string::npos) {
    return false;
  }

  // If fla annotations
  if (readAnnotate(f).find(attr) != std::string::npos) {
    return true;
  }

  // If fla flag is set
  if (flag == true) {
    /* Check if the number of applications is correct
    if (!((Percentage > 0) && (Percentage <= 100))) {
      LLVMContext &ctx = llvm::getGlobalContext();
      ctx.emitError(Twine("Flattening application function\
              percentage -perFLA=x must be 0 < x <= 100"));
    }
    // Check name
    else if (func.size() != 0 && func.find(f->getName()) != std::string::npos) {
      return true;
    }

    if ((((int)llvm::cryptoutils->get_range(100))) < Percentage) {
      return true;
    }
    */
    return true;
  }

  return false;
}
