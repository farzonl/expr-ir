#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/FormattedStream.h>

#include <iostream>
#include <string>
#include <vector>

using namespace llvm;

#define DEBUG 0

class expTree;
void makeLLVMExpModule(std::string stExpr, LLVMContext &context);
expTree *interpret(std::string stExpr);
void evalNode(expTree *node, IRBuilder<> &builder,
              Function::arg_iterator &args);
void evaluate(expTree *node);
void traverse(expTree *node);
void deleteTree(expTree *node);

LLVMContext cxt; // Note this can't go out of scope before mod.
std::unique_ptr<Module> mod;

class expTree {
public:
  expTree *left;
  expTree *right;
  std::string value;
  int varCount = 0;
  Value *eval;
  expTree(char value)
      : value(std::string(1, value)), left(nullptr), right(nullptr) {}

  expTree(std::string value) : value(value), left(nullptr), right(nullptr) {}

  expTree(std::string value, expTree *left, expTree *right)
      : value(value), left(left), right(right) {}

  expTree(char value, expTree *left, expTree *right)
      : value(std::string(1, value)), left(left), right(right) {}
};

void deleteTree(expTree *node) {
  if (node == nullptr) {
    return;
  }
  deleteTree(node->left);
  deleteTree(node->right);
  delete node;
  node = nullptr;
}

void traverse(expTree *node) {
  if (node == nullptr) {
    return;
  }
  traverse(node->right);
  traverse(node->left);
  std::cout << node->value << std::endl;
}

void makeLLVMExpModule(std::string stExpr, LLVMContext &context) {
  expTree *tree = interpret(stExpr);
#if DEBUG
  traverse(tree);
#endif
  // Module Construction
  mod = llvm::make_unique<Module>("exprFunc", context);
  evaluate(tree);
  deleteTree(tree);
}

expTree *interpret(std::string stExpr) {
  int varCount = 0;
  std::vector<expTree *> stack;
  for (int i = 0; i < stExpr.length(); i++) {
    switch (stExpr.at(i)) {
    case '+':
    case '-':
    case '/':
    case '*':
    case '|':
    case '&':
    case '^': {
      expTree *var1 = stack.back();
      stack.pop_back();
      expTree *var2 = stack.back();
      stack.pop_back();
      stack.push_back(new expTree(stExpr.at(i), var1, var2));
      break;
    }
    default: {
      varCount++;
      stack.push_back(new expTree(stExpr.at(i)));
    }
    }
  }
  stack[0]->varCount = varCount;
  return stack[0];
}

void evaluate(expTree *node) {
  if (node->varCount == 0) {
    return; // "please call on root node"
  }
  std::vector<Type *> funcArgs;
#if DEBUG
  std::cout << "var count: " << node->varCount << std::endl;
#endif
  for (int i = 0; i < node->varCount; i++) {
    funcArgs.push_back(Type::getInt32Ty(mod->getContext()));
  }

  FunctionType *funcType =
      FunctionType::get(Type::getInt32Ty(mod->getContext()), // ret type
                        funcArgs,                            // args
                        false);                              // isVarArg

  Function *expression =
      Function::Create(funcType,                     // Type
                       GlobalValue::ExternalLinkage, // Linkage
                       "expression",                 // Name
                       *mod.get());

  expression->setCallingConv(CallingConv::C);
#if DEBUG
  std::cout << "arg count: " << expression->arg_size() << std::endl;
#endif
  Function::arg_iterator args = expression->arg_begin();
  BasicBlock *block =
      BasicBlock::Create(mod->getContext(), "entry", expression);
  IRBuilder<> builder(block);
  evalNode(node, builder, args);
  builder.CreateRet(node->eval);
}

void evalNode(expTree *node, IRBuilder<> &builder,
              Function::arg_iterator &args) {
  if (node == nullptr) {
    return;
  }
  evalNode(node->right, builder, args);
  evalNode(node->left, builder, args);
  switch (node->value[0]) {
  case '+':
    node->eval = builder.CreateBinOp(Instruction::Add, node->left->eval,
                                     node->right->eval);
    break;
  case '-':
    node->eval = builder.CreateBinOp(Instruction::Sub, node->left->eval,
                                     node->right->eval);
    break;
  case '/':
    node->eval = builder.CreateBinOp(Instruction::UDiv, node->left->eval,
                                     node->right->eval);
    break;
  case '*':
    node->eval = builder.CreateBinOp(Instruction::Mul, node->left->eval,
                                     node->right->eval);
    break;
  case '|':
    node->eval = builder.CreateBinOp(Instruction::Or, node->left->eval,
                                     node->right->eval);
    break;
  case '&':
    node->eval = builder.CreateBinOp(Instruction::And, node->left->eval,
                                     node->right->eval);
    break;
  case '^':
    node->eval = builder.CreateBinOp(Instruction::Xor, node->left->eval,
                                     node->right->eval);
    break;
  default:
    node->eval = args++;
    node->eval->setName(node->value);
#if DEBUG
    std::cout << "node value: " << node->value << std::endl;
    outs() << "IR var name: " << node->eval->getName() << "\n";
#endif
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "usage: ./expr-ir <expression:(e.g.: \"ab+cde+**\")> "
              << std::endl;
    return -1;
  }
  makeLLVMExpModule(argv[1], cxt);
  verifyModule(*mod.get());

  legacy::PassManager PM;
  PM.add(createPrintModulePass(outs()));
  PM.run(*mod.get());

  return 0;
}