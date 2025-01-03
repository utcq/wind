#include <wind/generation/IR.h>

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

/**
 * @brief A class to optimize the intermediate representation (IR) of a program.
 */
class WindOptimizer {
public:
  /**
   * @brief Constructor for WindOptimizer.
   * @param program The IRBody representing the program to be optimized.
   */
  WindOptimizer(IRBody *program);

  /**
   * @brief Destructor for WindOptimizer.
   */
  virtual ~WindOptimizer();

  /**
   * @brief Gets the optimized IRBody.
   * @return The optimized IRBody.
   */
  IRBody *get();

private:
  IRBody *program;
  IRBody *emission;
  IRFunction *current_fn;

  /**
   * @brief Runs the optimization process on the program.
   */
  void optimize();

  /**
   * @brief Optimizes the body of a function.
   * @param body The body to be optimized.
   * @param parent The parent function of the body.
   */
  void OptimizeBody(IRBody *body, IRFunction *parent);

  /**
   * @brief Optimizes a generic node.
   * @param node The node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeNode(IRNode *node);

  /**
   * @brief Optimizes binary operations.
   * @param node The binary operation node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeBinOp(IRBinOp *node);

  /**
   * @brief Optimizes expressions.
   * @param node The expression node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeExpr(IRNode *node);

  /**
   * @brief Optimizes local variable declarations.
   * @param local_decl The local variable declaration node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeLDecl(IRVariableDecl *local_decl);

  /**
   * @brief Optimizes function calls.
   * @param fn_call The function call node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeFnCall(IRFnCall *fn_call);

  /**
   * @brief Optimizes functions.
   * @param fn The function node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeFunction(IRFunction *fn);

  /**
   * @brief Optimizes branching statements.
   * @param branch The branching node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeBranching(IRBranching *branch);

  /**
   * @brief Optimizes looping statements.
   * @param loop The looping node to be optimized.
   * @return An optimized IRNode.
   */
  IRNode *OptimizeLooping(IRLooping *loop);

  /**
   * @brief Optimizes constant folding for binary operations.
   * @param node The binary operation node to be optimized.
   * @return A new IRLiteral node with the optimized result.
   */
  IRLiteral *OptimizeConstFold(IRBinOp *node);
};

#endif