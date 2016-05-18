
#include "ExpressionUtil.hpp"
#include "ExpressionShape.hpp"

Expression add(Expression x, Expression y) {
  return {Expression::ADD, {x,y}};
}

Expression mul(Expression x, Expression y) {
  return {Expression::MUL, {x,y}};
}

Expression neg(Expression x) {
  return {Expression::NEG, {x}};
}

Expression var(int m, int n, int var_id) {
  auto attr = std::make_shared<VarAttributes>();
  attr->id = var_id;
  attr->size = {{m, n}};
  return {Expression::VAR, {}, attr};
}

Expression constant(double value) {
  auto attr = std::make_shared<ConstAttributes>();
  attr->dense_data = DenseMatrix::Constant(1, 1, value);
  return {Expression::CONST, {}, attr};
}

Expression quad_over_lin(Expression x, Expression y) {
  return {Expression::QUAD_OVER_LIN, {x,y}};
}

Expression p_norm(Expression x, double p) {
  return {Expression::P_NORM, {x}};
}

Expression hstack(std::vector<Expression> args) {
  return {Expression::HSTACK, args};
}

Expression vstack(std::vector<Expression> args) {
  return {Expression::VSTACK, args};
}

Expression reshape(Expression x, int m, int n) {
  auto attr = std::make_shared<ReshapeAttributes>();
  attr->size = {{m, n}};
  return {Expression::RESHAPE, {x}, attr};
}

Expression sum(Expression x) {
  return {Expression::SUM, {x}};
}

// x <= y
Expression leq(Expression x, Expression y) {
  return {Expression::LEQ, {x,y}};
}

// ||x||_2 <= y
Expression soc(Expression x, Expression y) {
  return {Expression::SOC, {x,y}};
}

Expression epi_var(const Expression& x, const std::string& name) {
  Size size_x = size(x);
  return var(size_x.dims[0], size_x.dims[1], rand());
}

Expression scalar_epi_var(const Expression& x, const std::string& name) {
  return var(1, 1, rand());
}

int count_nodes(const Expression& x) {
  int retval = 1;
  for (const Expression& arg : x.args()) {
    retval += count_nodes(arg);
  }
  return retval;
}

int count_nodes(const Problem& problem) {
  int retval = count_nodes(problem.objective);
  for (const Expression& constr : problem.constraints) {
    retval += count_nodes(constr);
  }
  return retval;
}

bool is_scalar(const Size& size) {
  return size.dims[0] == 1 && size.dims[1] == 1;
}
