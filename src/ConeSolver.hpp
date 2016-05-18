// Interface for cone solvers
//
// TODO(mwytock): This should likely be a pure C interface to ease
// implementation for existing cone solvers (ECOS, SCS)

#ifndef CONE_SOLVER_H
#define CONE_SOLVER_H

#include <Eigen/Core>
#include <Eigen/Sparse>

#include "Utils.hpp"

class ConeConstraint {
 public:
  enum Cone {
    ZERO,
    NON_NEGATIVE,
    SECOND_ORDER,
  };

  Cone cone;
  int offset, size;
};

// A cone problem of the form
// minimize    c'x
// subject to  b - Ax in K
class ConeProblem {
 public:
  SparseMatrix A;
  DenseVector b, c;
  std::vector<ConeConstraint> constraints;
};

class ConeSolution {
 public:
  DenseVector x, y;
  double objective_value;
};

class ConeSolver {
 public:
  virtual ~ConeSolver() {}
  virtual ConeSolution solve(const ConeProblem& problem) = 0;
};

#endif  // CONE_SOLVER_H
