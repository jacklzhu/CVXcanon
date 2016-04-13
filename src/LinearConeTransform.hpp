
#ifndef LINEAR_CONE_TRANSFORM_H
#define LINEAR_CONE_TRANSFORM_H

#include "ProblemTransform.hpp"

class LinearConeTransform : public ProblemTransform {
 public:
  virtual Problem transform(const Problem& problem);
};

#endif  // LINEAR_CONE_TRANSFORM_H