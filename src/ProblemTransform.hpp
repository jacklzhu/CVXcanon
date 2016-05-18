
#ifndef PROBLEM_TRANSFORM_H
#define PROBLEM_TRANSFORM_H

#include "Expression.hpp"

class ProblemTransform {
 public:
  virtual ~ProblemTransform() {}
  virtual Problem transform(const Problem& problem) = 0;
};

#endif  // PROBLEM_TRANSFORM_H
