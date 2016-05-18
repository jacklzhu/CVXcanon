
#include "SymbolicConeSolver.hpp"

#include <memory>
#include <unordered_map>

#include "ExpressionShape.hpp"
#include "ExpressionUtil.hpp"
#include "LinearExpression.hpp"
#include "MatrixUtil.hpp"
#include "Utils.hpp"

const std::unordered_map<Expression::Type, ConeConstraint::Cone> kConeMap = {
  {Expression::EQ,  ConeConstraint::ZERO},
  {Expression::LEQ, ConeConstraint::NON_NEGATIVE},
  {Expression::SOC, ConeConstraint::SECOND_ORDER},
};

SymbolicConeSolver::SymbolicConeSolver(
    std::unique_ptr<ConeSolver> cone_solver)
    : cone_solver_(std::move(cone_solver)),
      num_constrs_(0) {}

int SymbolicConeSolver::add_constraint_coefficients(const Expression& expr) {
  CoeffMap coeff_map = get_coefficients(expr);

  if (is_constant(coeff_map)) {
    // Drop constant constraints
    // TODO(mwytock): Correct way to deal with this case?
    return 0;
  }

  const int m = dim(expr);

  for (const auto& iter : coeff_map) {
    const int var_id = iter.first;
    const SparseMatrix& A = iter.second;
    if (var_id == kConstCoefficientId) {
      append_block_triplets(A, num_constrs_, 0, &b_coeffs_);
    } else {
      const int j = var_offsets_.insert(var_id, A.cols());
      append_block_triplets(A, num_constrs_, j, &A_coeffs_);
    }
  }

  num_constrs_ += m;
  return m;
}

void SymbolicConeSolver::add_cone_constraint(const Expression& expr) {
  int offset = num_constrs_;
  int size = 0;
  if (expr.type() == Expression::EQ ||
      expr.type() == Expression::LEQ) {
    // x == y becomes y - x in K_0
    // x <= y becomes y - x in K_+
    assert(expr.args().size() == 2);
    size += add_constraint_coefficients(add(neg(expr.arg(0)), expr.arg(1)));
  } else if (expr.type() == Expression::SOC) {
    // ||x||_2 <= y becomes (y,x) in K_soc
    assert(expr.args().size() == 2);
    size += add_constraint_coefficients(expr.arg(1));
    size += add_constraint_coefficients(expr.arg(0));
  } else {
    assert(false);
  }

  if (size != 0) {
    ConeConstraint constr = {find_or_die(kConeMap, expr.type()), offset, size};
    cone_problem_.constraints.push_back(constr);
  }
}

void SymbolicConeSolver::add_cone_objective(const Expression& expr) {
  CoeffMap coeff_map = get_coefficients(expr);
  for (const auto& iter : coeff_map) {
    const int var_id = iter.first;
    const SparseMatrix& cT = iter.second;

    if (var_id != kConstCoefficientId) {
      const int j = var_offsets_.insert(var_id, cT.cols());
      append_block_triplets(cT.transpose(), j, 0, &c_coeffs_);
    }
  }
}

Solution SymbolicConeSolver::get_solution(
    const ConeSolution& cone_solution) {
  Solution solution;
  for (const auto& iter : var_offsets_.map()) {
    const int var_id = iter.first;
    const int j = iter.second.first;
    const int n = iter.second.second;
    solution.values[var_id] = cone_solution.x.segment(j, n);
  }
  solution.objective_value = cone_solution.objective_value;
  return solution;
}

Solution SymbolicConeSolver::solve(const Problem& problem) {
  add_cone_objective(problem.objective);
  for (const Expression& constr : problem.constraints)
    add_cone_constraint(constr);

  const int m = num_constrs_;
  const int n = var_offsets_.n();
  cone_problem_.A = -sparse_matrix(m, n, A_coeffs_);  // negative by convention
  cone_problem_.b = sparse_matrix(m, 1, b_coeffs_);
  cone_problem_.c = sparse_matrix(n, 1, c_coeffs_);

  printf("created cone problem matrices (m=%d, n=%d)\n", m, n);
  printf("A:\n%s", matrix_debug_string(cone_problem_.A).c_str());
  printf("b: %s\n", vector_debug_string(cone_problem_.b).c_str());
  printf("c: %s\n", vector_debug_string(cone_problem_.c).c_str());
  printf("\n");

  return get_solution(cone_solver_->solve(cone_problem_));
}
