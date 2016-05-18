
#include "SplittingConeSolver.hpp"

#include <unordered_map>

#include "MatrixUtil.hpp"

namespace scs {
#include <scs/include/util.h>
}

void SplittingConeSolver::build_scs_constraint(
    const Eigen::SparseMatrix<double, Eigen::RowMajor>& A,
    const DenseVector& b,
    const std::vector<ConeConstraint>& constraints,
    int* total_size,
    int* sizes) {
  int j = 0;
  for (const ConeConstraint& constr : constraints) {
    append_block_triplets(
        A.middleRows(constr.offset, constr.size), num_constrs_, 0, &A_coeffs_);
    b_.segment(num_constrs_, constr.size) =
        b.segment(constr.offset, constr.size);
    if (total_size != nullptr) *total_size += constr.size;
    if (sizes != nullptr) sizes[j++] = constr.size;
    num_constrs_ += constr.size;
  }
}

void SplittingConeSolver::build_scs_problem(
    const ConeProblem& problem,
    ConeSolution* solution) {
  const int m = problem.A.rows();
  const int n = problem.A.cols();

  // Inputs: problem.A, problem.b -  original constraints
  // Outputs: A_, b_ - with shuffled rows, will build incrementally
  {
    Eigen::SparseMatrix<double, Eigen::RowMajor> A = problem.A;
    const DenseVector& b = problem.b;

    A_coeffs_.clear();
    b_ = DenseVector(m);
    num_constrs_ = 0;

    std::unordered_map<int, std::vector<ConeConstraint>> constr_map;
    for (const ConeConstraint& constr : problem.constraints) {
      constr_map[constr.cone].push_back(constr);
    }

    build_scs_constraint(
        A, b, constr_map[ConeConstraint::ZERO], &cone_.f, nullptr);
    build_scs_constraint(
        A, b, constr_map[ConeConstraint::NON_NEGATIVE], &cone_.l, nullptr);
    cone_.qsize = constr_map[ConeConstraint::SECOND_ORDER].size();
    if (cone_.qsize != 0) {
      q_.reset(new int[cone_.qsize]);
      build_scs_constraint(
          A, b, constr_map[ConeConstraint::SECOND_ORDER], nullptr, q_.get());
      cone_.q = q_.get();
    }
    cone_.ssize = 0;
    cone_.ep = 0;
    cone_.ed = 0;
    cone_.psize = 0;

    A_ = sparse_matrix(m, n, A_coeffs_);
  }

  // printf("SCS constraints:\n");
  // printf("A:\n%s", matrix_debug_string(A_).c_str());
  // printf("b: %s\n", vector_debug_string(b_).c_str());

  // Build SCS data structures
  A_matrix_.m = m;
  A_matrix_.n = n;
  A_matrix_.p = A_.outerIndexPtr();
  A_matrix_.i = A_.innerIndexPtr();
  A_matrix_.x = A_.valuePtr();

  data_.m = m;
  data_.n = n;
  data_.A = &A_matrix_;
  data_.b = const_cast<double*>(b_.data());
  data_.c = const_cast<double*>(problem.c.data());
  data_.stgs = &settings_;
  scs::setDefaultSettings(&data_);

  s_ = DenseVector(A_.rows());
  solution->x = DenseVector(A_.cols());
  solution->y = DenseVector(A_.rows());
  sol_.x = const_cast<double*>(solution->x.data());
  sol_.y = const_cast<double*>(solution->y.data());
  sol_.s = const_cast<double*>(s_.data());
}

ConeSolution SplittingConeSolver::solve(const ConeProblem& problem) {
  ConeSolution solution;
  build_scs_problem(problem, &solution);
  scs::scs(&data_, &cone_, &sol_, &info_);
  solution.objective_value = info_.pobj;
  return solution;
}
