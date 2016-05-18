
#include "Solver.hpp"

#include "LinearConeTransform.hpp"
#include "SplittingConeSolver.hpp"
#include "SymbolicConeSolver.hpp"
#include "TextFormat.hpp"

Solution solve(const Problem& problem, const SolverOptions& solver_options) {
  // TODO(mwytock): Allow for different transforms/solvers as per SolveOptions

  printf("input problem:\n%s\n\n", format_problem(problem).c_str());
  LinearConeTransform transform;
  Problem cone_problem = transform.transform(problem);

  printf("cone problem:\n%s\n\n", format_problem(cone_problem).c_str());
  SymbolicConeSolver solver(std::make_unique<SplittingConeSolver>());
  return solver.solve(cone_problem);
}
