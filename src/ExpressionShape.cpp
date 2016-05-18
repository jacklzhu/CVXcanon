
#include "ExpressionShape.hpp"
#include "ExpressionUtil.hpp"
#include "TextFormat.hpp"

#include <unordered_map>

Size get_add_shape(const Expression& expr) {
  // TODO(mwytock): promotion
  return size(expr.arg(0));
}

Size get_sum_shape(const Expression& expr) {
  // TODO(mwytock): axis parameter
  return {{1,1}};
}

Size get_mul_shape(const Expression& expr) {
  assert(expr.args().size() == 2);
  Size lhs_size = size(expr.arg(0));
  Size rhs_size = size(expr.arg(1));

  // promotion
  if (is_scalar(lhs_size))
    return rhs_size;
  if (is_scalar(rhs_size))
    return lhs_size;

  assert(lhs_size.dims[1] == rhs_size.dims[0]);
  return {{lhs_size.dims[0], rhs_size.dims[1]}};
}

Size get_const_shape(const Expression& expr) {
  return expr.attr<ConstAttributes>().size();
}

Size get_var_shape(const Expression& expr) {
  return expr.attr<VarAttributes>().size;
}

Size get_hstack_shape(const Expression& expr) {
  int cols = 0;
  for (const Expression& arg : expr.args())
    cols += size(arg).dims[1];
  return {{size(expr.arg(0)).dims[0], cols}};
}

Size get_vstack_shape(const Expression& expr) {
  int rows = 0;
  for (const Expression& arg : expr.args())
    rows += size(arg).dims[0];
  return {{rows, size(expr.arg(0)).dims[1]}};
}

Size get_reshape_shape(const Expression& expr) {
  return expr.attr<ReshapeAttributes>().size;
}

typedef Size(*ShapeFunction)(const Expression& expr);

std::unordered_map<int, ShapeFunction> kShapeFunctions = {
  {Expression::ADD, &get_add_shape},
  {Expression::CONST, &get_const_shape},
  {Expression::HSTACK, &get_hstack_shape},
  {Expression::MUL, &get_mul_shape},
  {Expression::RESHAPE, &get_reshape_shape},
  {Expression::SUM, &get_sum_shape},
  {Expression::VAR, &get_var_shape},
  {Expression::VSTACK, &get_vstack_shape},
};

Size size(const Expression& expr) {
  // TODO(mwytock): Add caching (w/ storage on Expression) so that we dont
  // repeatedly recompute size for the same node.
  auto iter = kShapeFunctions.find(expr.type());
  if (iter == kShapeFunctions.end()) {
    assert(expr.args().size() == 1);
    return size(expr.arg(0));
  }
  Size retval = iter->second(expr);
  // printf("size: %d x %d, %s\n", retval.dims[0], retval.dims[1],
  //        format_expression(expr).c_str());
  return retval;
}

int dim(const Expression& expr) {
  Size s = size(expr);
  return s.dims[0]*s.dims[1];
}
