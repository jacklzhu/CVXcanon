// Builds matrices for the canonical cone form and passes to ConeSolver

#ifndef SYMBOLIC_CONE_SOLVER_H
#define SYMBOLIC_CONE_SOLVER_H

#include "ConeSolver.hpp"
#include "Solver.hpp"

class VariableOffsetMap {
 public:
  VariableOffsetMap() : n_(0) {}

  // returns offset, inserting if necssary
  int insert(int var_id, int size) {
    auto ret = map_.insert(std::make_pair(var_id, std::make_pair(n_, size)));
    if (ret.second)
      n_ += size;
    return ret.first->second.first;
  }

  int n() const { return n_; }
  const std::map<int, std::pair<int, int>>& map() const { return map_; }

 private:
  int n_;

  // var_id -> (offset, size)
  std::map<int, std::pair<int, int>> map_;
};

class SymbolicConeSolver : public Solver {
 public:
  SymbolicConeSolver(std::unique_ptr<ConeSolver> solver);
  Solution solve(const Problem& problem) override;

 private:
  void add_cone_objective(const Expression& expr);
  void add_cone_constraint(const Expression& expr);
  int add_constraint_coefficients(const Expression& expr);
  Solution get_solution(const ConeSolution& cone_solution);

  std::unique_ptr<ConeSolver> cone_solver_;

  // For building ConeProblem data
  VariableOffsetMap var_offsets_;
  ConeProblem cone_problem_;
  std::vector<Triplet> A_coeffs_, b_coeffs_, c_coeffs_;
  int num_constrs_;
};

#endif  // SYMBOLIC_CONE_SOLVER_H
