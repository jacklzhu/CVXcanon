// Interface to the splitting conic solver (SCS)
//
// See: https://github.com/cvxgrp/scs

#ifndef SPLITTING_CONIC_SOLVER_H
#define SPLITTING_CONIC_SOLVER_H

#include "ConeSolver.hpp"

namespace scs {
typedef double scs_float;
typedef int scs_int;
#include <scs/linsys/amatrix.h>
#include <scs/include/scs.h>
}  // scs

class SplittingConeSolver : public ConeSolver {
 public:
  ConeSolution solve(const ConeProblem& problem) override;

 private:
  void build_scs_problem(
      const ConeProblem& problem,
      ConeSolution* solution);

  void build_scs_constraint(
    const Eigen::SparseMatrix<double, Eigen::RowMajor>& A,
    const DenseVector& b,
    const std::vector<ConeConstraint>& constraints,
    int* total_size,
    int* sizes);

  // SCS data structures
  scs::Data data_;
  scs::Cone cone_;
  scs::Info info_;
  scs::Sol sol_;
  scs::Settings settings_;

  // SCS supporting data structures
  scs::AMatrix A_matrix_;
  std::unique_ptr<int[]> q_;
  DenseVector s_;

  // Constraints ordered the way SCS needs them
  SparseMatrix A_;
  DenseVector b_;

  // Used for building constraints
  int num_constrs_;
  std::vector<Triplet> A_coeffs_;

};

#endif  // SPLITTING_CONIC_SOLVER_H
