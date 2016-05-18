#ifndef TEXT_FORMAT_H
#define TEXT_FORMAT_H

#include <string>

#include "Expression.hpp"

std::string format_expression(const Expression& expr);
std::string format_problem(const Problem& problem);

#endif  // TEXT_FORMAT_H
