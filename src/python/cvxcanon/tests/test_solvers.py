"""Test solvers called from CVXcanon.

TODO(mwytock): Add comprehensive tests, all solvers, etc.
"""

import cvxpy as cp
import numpy as np
import cvxcanon

def test_lasso():
    np.random.seed(0)

    m = 5
    n = 10
    lam = 1

    A = np.random.randn(m,n)
    b = np.random.randn(m)
    x = cp.Variable(n)
    f = cp.sum_squares(A*x - b) + lam*cp.norm1(x)
    prob = cp.Problem(cp.Minimize(f))

    result0 = prob.solve()
    result1 = cvxcanon.solve(prob)

    tol = 1e-2
    assert -tol <= (result1 - result0)/(1+np.abs(result0)) <= tol
