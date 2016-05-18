
#ifndef LINEAR_CONE_TRANSFORM_H
#define LINEAR_CONE_TRANSFORM_H

#include "ProblemTransform.hpp"

class LinearConeTransform : public ProblemTransform {
 public:
  Problem transform(const Problem& problem) override;
};

#endif  // LINEAR_CONE_TRANSFORM_H
