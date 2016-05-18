// Interface for all solvers

#ifndef SOLVER_H
#define SOLVER_H

#include <map>

#include <Eigen/Core>

#include "Expression.hpp"

class SolverOptions {
};

class Solution {
 public:
  std::map<int, DenseVector> values;
  double objective_value;
};

class Solver {
 public:
  virtual ~Solver() {}
  virtual Solution solve(const Problem& problem) = 0;
};

Solution solve(const Problem& problem, const SolverOptions& solver_options);

#endif  // SOLVER_H
