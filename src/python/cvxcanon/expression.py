"""Handles translating CVXPY expressions to CVXcanon expressions."""

import numpy as np

import CVXcanon

import cvxpy
import cvxpy.atoms.affine.add_expr
import cvxpy.atoms.affine.binary_operators
import cvxpy.atoms.affine.unary_operators
import cvxpy.atoms.pnorm
import cvxpy.atoms.quad_over_lin
import cvxpy.expressions.constants.constant
import cvxpy.expressions.variables.variable


SENSE_MAP = {
    cvxpy.problems.objective.Maximize: CVXcanon.Problem.MAXIMIZE,
    cvxpy.problems.objective.Minimize: CVXcanon.Problem.MINIMIZE,
}

TYPE_MAP = {
    cvxpy.atoms.affine.add_expr.AddExpression: CVXcanon.Expression.ADD,
    cvxpy.atoms.affine.binary_operators.MulExpression: CVXcanon.Expression.MUL,
    cvxpy.atoms.affine.unary_operators.NegExpression: CVXcanon.Expression.NEG,
    cvxpy.atoms.pnorm: CVXcanon.Expression.P_NORM,
    cvxpy.atoms.quad_over_lin: CVXcanon.Expression.QUAD_OVER_LIN,
    cvxpy.constraints.eq_constraint.EqConstraint: CVXcanon.Expression.EQ,
    cvxpy.constraints.leq_constraint.LeqConstraint: CVXcanon.Expression.LEQ,
    cvxpy.expressions.constants.constant.Constant: CVXcanon.Expression.CONST,
    cvxpy.expressions.variables.variable.Variable: CVXcanon.Expression.VAR,
}

def get_var_attributes(variable):
    attr = CVXcanon.VarAttributes()
    attr.id = variable.id
    attr.size.dims.push_back(variable.size[0])
    attr.size.dims.push_back(variable.size[1])
    return attr

def get_const_attributes(constant):
    attr = CVXcanon.ConstAttributes()
    attr.set_dense_data(np.asfortranarray(np.atleast_2d(constant.value)))
    return attr

def get_pnorm_attributes(pnorm):
    attr = CVXcanon.PNormAttributes()
    attr.p = pnorm.p
    return attr

ATTRIBUTE_MAP = {
    cvxpy.expressions.constants.constant.Constant: get_const_attributes,
    cvxpy.expressions.variables.variable.Variable: get_var_attributes,
    cvxpy.atoms.pnorm: get_pnorm_attributes,
}

def convert_expression(cvxpy_expr):
    args = CVXcanon.ExpressionVector()
    for arg in cvxpy_expr.args:
        args.push_back(convert_expression(arg))
    t = type(cvxpy_expr)
    expr = CVXcanon.Expression(
        TYPE_MAP[t],
        args,
        ATTRIBUTE_MAP.get(t, lambda x: None)(cvxpy_expr))
    return expr

def convert_problem(cvxpy_problem):
    problem = CVXcanon.Problem()
    problem.sense = SENSE_MAP[type(cvxpy_problem.objective)]
    problem.objective = convert_expression(cvxpy_problem.objective.args[0])
    for cvxpy_constr in cvxpy_problem.constraints:
        problem.constraints.push_back(convert_expression(cvxpy_constr))
    return problem
