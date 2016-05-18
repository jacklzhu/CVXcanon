#include "LinearExpression.hpp"

#include <unordered_map>

#include "ExpressionUtil.hpp"
#include "ExpressionShape.hpp"
#include "MatrixUtil.hpp"
#include "TextFormat.hpp"

bool is_constant(const CoeffMap& coeffs) {
  return (coeffs.find(kConstCoefficientId) != coeffs.end() &&
          coeffs.size() == 1);
}

std::vector<SparseMatrix> get_add_coefficients(const Expression& expr) {
  std::vector<SparseMatrix> coeffs;
  for (int i = 0; i < expr.args().size(); i++)
    coeffs.push_back(scalar_matrix(1, dim(expr)));
  return coeffs;
}

std::vector<SparseMatrix> get_neg_coefficients(const Expression& expr) {
  return {scalar_matrix(-1, dim(expr))};
}

std::vector<SparseMatrix> get_sum_coefficients(const Expression& expr) {
  return {ones_matrix(1, dim(expr.arg(0)))};
}

std::vector<SparseMatrix> get_stack_coefficients(const Expression& expr, bool vertical) {
  std::vector<SparseMatrix> coeffs;
  int offset = 0;
  Size expr_size = size(expr);
  for (const Expression& arg : expr.args()) {
    Size arg_size = size(arg);

    /* If VERTICAL, columns that are interleaved. Otherwise, they are
       laid out in order. */
    int column_offset;
    int offset_increment;
    if (vertical) {
      column_offset = expr_size.dims[0];
      offset_increment = arg_size.dims[0];
    } else {
      column_offset = arg_size.dims[0];
      offset_increment = dim(arg);
    }

    std::vector<Triplet> arg_coeffs;
    arg_coeffs.reserve(dim(arg));
    for (int i = 0; i < arg_size.dims[0]; i++) {
      for (int j = 0; j < arg_size.dims[1]; j++) {
        int row_idx = i + (j * column_offset) + offset;
        int col_idx = i + (j * arg_size.dims[0]);
        arg_coeffs.push_back(Triplet(row_idx, col_idx, 1));
      }
    }

    coeffs.push_back(sparse_matrix(dim(expr), dim(arg), arg_coeffs));
    offset += offset_increment;
  }
  return coeffs;
}

std::vector<SparseMatrix> get_hstack_coefficients(const Expression& expr) {
  return get_stack_coefficients(expr, false);
}

std::vector<SparseMatrix> get_vstack_coefficients(const Expression& expr) {
  return get_stack_coefficients(expr, true);
}

std::vector<SparseMatrix> get_reshape_coefficients(const Expression& expr) {
  return {identity(dim(expr))};
}

typedef std::vector<SparseMatrix>(*CoefficientFunction)(
    const Expression& expr);

std::unordered_map<int, CoefficientFunction> kCoefficientFunctions = {
  {Expression::ADD, &get_add_coefficients},
  {Expression::NEG, &get_neg_coefficients},
  {Expression::SUM, &get_sum_coefficients},
  {Expression::HSTACK, &get_hstack_coefficients},
  {Expression::RESHAPE, &get_reshape_coefficients},
  {Expression::VSTACK, &get_vstack_coefficients},
};

// result += lhs*rhs
void multiply_by_constant(
    const SparseMatrix& lhs,
    const std::map<int, Matrix>& rhs,
    std::map<int, SparseMatrix>* result) {
  for (const auto& iter : rhs) {
    SparseMatrix value = lhs*iter.second;
    auto ret = result->insert(std::make_pair(iter.first, value));
    if (!ret.second)
      ret.first->second += value;
  }
}

SparseMatrix promote_multiply(const SparseMatrix& A, int m, int n) {
  if (A.rows() == 1 && A.cols() == 1) {
    assert(m == n);
    return scalar_matrix(A.coeff(0,0), n);
  }

  return reshape(A, m, n);
}

std::map<int, SparseMatrix> get_coefficients(const Expression& expr) {
  std::map<int, SparseMatrix> coeffs;

  if (expr.type() == Expression::CONST) {
    coeffs[kConstCoefficientId] = to_vector(
        expr.attr<ConstAttributes>().dense_data).sparseView();
  } else if (expr.type() == Expression::VAR) {
    coeffs[expr.attr<VarAttributes>().id] = identity(dim(expr));
  } else if (expr.type() == Expression::MUL) {
    // Special case for binary mul operator which is guaranteed to have one
    // constant argument by DCP rules.
    assert(expr.args().size() == 2);
    CoeffMap lhs_coeffs = get_coefficients(expr.arg(0));
    CoeffMap rhs_coeffs = get_coefficients(expr.arg(1));
    std::vector<SparseMatrix> f_coeffs;
    if (is_constant(lhs_coeffs)) {
      // TODO(mwytock): matrix-matrix multiply not yet supported
      assert(size(expr).dims[1] == 1);
      SparseMatrix f_coeffs = promote_multiply(
          lhs_coeffs[kConstCoefficientId],
          size(expr).dims[0],
          size(expr.arg(1)).dims[0]);
      multiply_by_constant(f_coeffs, rhs_coeffs, &coeffs);
    } else if (is_constant(rhs_coeffs)) {
      assert(false);
    } else {
      assert(false);
    }
  } else {
    auto iter = kCoefficientFunctions.find(expr.type());
    assert(iter != kCoefficientFunctions.end());
    std::vector<Matrix> f_coeffs = iter->second(expr);
    for (int i = 0; i < expr.args().size(); i++) {
      multiply_by_constant(
          f_coeffs[i], get_coefficients(expr.arg(i)), &coeffs);
    }
  }

  // debugging
  // printf("get_coefficients: %s\n", format_expression(expr).c_str());
  // for (const auto& iter : coeffs)
  //   printf("id: %d\n%s", iter.first, matrix_debug_string(iter.second).c_str());
  // printf("\n");

  return coeffs;
}
