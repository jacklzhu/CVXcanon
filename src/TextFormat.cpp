
#include "TextFormat.hpp"

#include <unordered_map>

#include "Expression.hpp"

std::unordered_map<int, std::string> kSenseNames = {
  {Problem::MAXIMIZE, "maximize"},
  {Problem::MINIMIZE, "minimize"},
};

std::unordered_map<int, std::string> kExpressionNames = {
  {Expression::ADD, "add"},
  {Expression::CONST, "const"},
  {Expression::EQ, "eq"},
  {Expression::HSTACK, "hstack"},
  {Expression::INDEX, "index"},
  {Expression::LEQ, "leq"},
  {Expression::MUL, "mul"},
  {Expression::NEG, "neg"},
  {Expression::P_NORM, "p_norm"},
  {Expression::QUAD_OVER_LIN, "quad_over_lin"},
  {Expression::RESHAPE, "reshape"},
  {Expression::SUM, "sum"},
  {Expression::VAR, "var"},
  {Expression::VSTACK, "vstack"},
  {Expression::SOC, "soc"},
};

std::string get_name(
    const std::unordered_map<int, std::string>& names,
    int value) {
  auto iter = names.find(value);
  if (iter != names.end()) {
    return iter->second;
  } else {
    return "unknown" + std::to_string(value);
  }
}

std::string format_expression(const Expression& expr) {
  std::string retval = get_name(kExpressionNames, expr.type());
  if (!expr.args().empty()) {
    retval += "(";
    for (int i = 0; i < expr.args().size(); i++) {
      if (i != 0) retval += ", ";
      retval += format_expression(expr.arg(i));
    }
    retval += ")";
  }
  return retval;
}

std::string format_problem(const Problem& problem) {
  std::string retval;
  retval += get_name(kSenseNames, problem.sense);
  retval += "   ";
  retval += format_expression(problem.objective);
  if (!problem.constraints.empty()) {
    retval += "\nsubject to ";
    for (int i = 0; i < problem.constraints.size(); i++) {
      if (i != 0) retval += "\n           ";
      retval += format_expression(problem.constraints[i]);
    }
  }
  return retval;
}
