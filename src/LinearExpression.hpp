// Methods for converting linear expressions to sparse matrices
//
// TODO(mwytock): Merge all functionality from LinOpOperations and replace LinOp
// with Expression everywhere.

#ifndef LINEAR_EXPRESSION_H
#define LINEAR_EXPRESSION_H

#include <map>

#include "Utils.hpp"
#include "Expression.hpp"

const int kConstCoefficientId = -1;
std::map<int, SparseMatrix> get_coefficients(const Expression& expr);
bool is_constant(const CoeffMap& coeffs);

#endif  // LINEAR_EXPRESSION_H
