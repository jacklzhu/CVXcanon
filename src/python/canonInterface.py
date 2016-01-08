#    This file is part of CVXcanon.
#
#    CVXcanon is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    CVXcanon is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with CVXcanon.  If not, see <http:#www.gnu.org/licenses/>.

import CVXcanon
import numpy as np
import scipy.sparse
from collections import deque
from cvxpy.lin_ops.lin_op import *
from cvxpy.lin_ops import LinEqConstr, LinLeqConstr
from cvxpy.constraints import SOC, SDP, ExpCone
from cvxpy.problems.objective import Minimize, Maximize


def get_problem_matrix(constrs, id_to_col=None, constr_offsets=None):
    '''
    Builds a sparse representation of the problem data by calling CVXCanon's
    C++ build_matrix function.

    Parameters
    ----------
        constrs: A list of python linOp trees
        id_to_col: A map from variable id to offset within our matrix

    Returns
    ----------
        V, I, J: numpy arrays encoding a sparse representation of our problem
        const_vec: a numpy column vector representing the constant_data in our problem
    '''
    linOps = [constr.expr for constr in constrs]
    lin_vec = CVXcanon.LinOpVector()

    id_to_col_C = CVXcanon.IntIntMap()
    if id_to_col is None:
        id_to_col = {}

    # Loading the variable offsets from our
    # Python map into a C++ map
    for id, col in id_to_col.items():
        id_to_col_C[int(id)] = int(col)

    # This array keeps variables data in scope
    # after build_lin_op_tree returns
    tmp = []
    for lin in linOps:
        tree = build_lin_op_tree(lin, tmp)
        tmp.append(tree)
        lin_vec.push_back(tree)

    if constr_offsets is None:
        problemData = CVXcanon.build_matrix(lin_vec, id_to_col_C)
    else:
        # Load constraint offsets into a C++ vector
        constr_offsets_C = CVXcanon.IntVector()
        for offset in constr_offsets:
            constr_offsets_C.push_back(int(offset))
        problemData = CVXcanon.build_matrix(lin_vec, id_to_col_C,
                                            constr_offsets_C)

    # Unpacking
    V = problemData.getV(len(problemData.V))
    I = problemData.getI(len(problemData.I))
    J = problemData.getJ(len(problemData.J))
    const_vec = problemData.getConstVec(len(problemData.const_vec))

    return V, I, J, const_vec.reshape(-1, 1)


def format_matrix(matrix, format='dense'):
    ''' Returns the matrix in the appropriate form,
        so that it can be efficiently loaded with our swig wrapper
    '''
    if (format == 'dense'):
        # Ensure is 2D.
        matrix = np.atleast_2d(matrix)
        return np.asfortranarray(matrix)
    elif(format == 'sparse'):
        return scipy.sparse.coo_matrix(matrix)
    elif(format == 'scalar'):
        return np.asfortranarray(np.matrix(matrix))
    else:
        raise NotImplementedError()


def set_matrix_data(linC, linPy):
    '''  Calls the appropriate CVXCanon function to set the matrix data field of our C++ linOp.
    '''
    if isinstance(linPy.data, LinOp):
        if linPy.data.type == 'sparse_const':
            coo = format_matrix(linPy.data.data, 'sparse')
            linC.set_sparse_data(coo.data, coo.row.astype(float),
                                 coo.col.astype(float), coo.shape[0], coo.shape[1])
        elif linPy.data.type == 'dense_const':
            linC.set_dense_data(format_matrix(linPy.data.data))
        else:
            raise NotImplementedError()
    else:
        if linPy.type == 'sparse_const':
            coo = format_matrix(linPy.data, 'sparse')
            linC.set_sparse_data(coo.data, coo.row.astype(float),
                                 coo.col.astype(float), coo.shape[0], coo.shape[1])
        else:
            linC.set_dense_data(format_matrix(linPy.data))


def set_slice_data(linC, linPy):
    '''
    Loads the slice data, start, stop, and step into our C++ linOp.
    The semantics of the slice operator is treated exactly the same as in Python.
    Note that the 'None' cases had to be handled at the wrapper level, since we must load
    integers into our vector.
    '''
    for i, sl in enumerate(linPy.data):
        vec = CVXcanon.IntVector()

        start = 0
        if sl.start is not None:
            start = sl.start

        stop = linPy.args[0].size[i]
        if sl.stop is not None:
            stop = sl.stop

        step = 1
        if sl.step is not None:
            step = sl.step

        # handle [::-1] case
        if step < 0 and sl.start is None and sl.stop is None:
            tmp = start
            start = stop - 1
            stop = tmp

        for var in [start, stop, step]:
            vec.push_back(var)

        linC.slice.push_back(vec)


type_map = {
    "VARIABLE": CVXcanon.VARIABLE,
    "PROMOTE": CVXcanon.PROMOTE,
    "MUL": CVXcanon.MUL,
    "RMUL": CVXcanon.RMUL,
    "MUL_ELEM": CVXcanon.MUL_ELEM,
    "DIV": CVXcanon.DIV,
    "SUM": CVXcanon.SUM,
    "NEG": CVXcanon.NEG,
    "INDEX": CVXcanon.INDEX,
    "TRANSPOSE": CVXcanon.TRANSPOSE,
    "SUM_ENTRIES": CVXcanon.SUM_ENTRIES,
    "TRACE": CVXcanon.TRACE,
    "RESHAPE": CVXcanon.RESHAPE,
    "DIAG_VEC": CVXcanon.DIAG_VEC,
    "DIAG_MAT": CVXcanon.DIAG_MAT,
    "UPPER_TRI": CVXcanon.UPPER_TRI,
    "CONV": CVXcanon.CONV,
    "HSTACK": CVXcanon.HSTACK,
    "VSTACK": CVXcanon.VSTACK,
    "SCALAR_CONST": CVXcanon.SCALAR_CONST,
    "DENSE_CONST": CVXcanon.DENSE_CONST,
    "SPARSE_CONST": CVXcanon.SPARSE_CONST,
    "NO_OP": CVXcanon.NO_OP,
    "KRON": CVXcanon.KRON,
}


def get_type(ty):
    if ty in type_map:
        return type_map[ty]
    else:
        raise NotImplementedError()

def build_lin_op_tree(root_linPy, tmp):
    '''
    Breadth-first, pre-order traversal on the Python linOp tree
    Parameters
    -------------
    root_linPy: a Python LinOp tree

    tmp: an array to keep data from going out of scope

    Returns
    --------
    root_linC: a C++ LinOp tree created through our swig interface
    '''
    Q = deque()
    root_linC = CVXcanon.LinOp()
    Q.append((root_linPy, root_linC))

    while len(Q) > 0:
        linPy, linC = Q.popleft()

        # Updating the arguments our LinOp
        for argPy in linPy.args:
            tree = CVXcanon.LinOp()
            tmp.append(tree)
            Q.append((argPy, tree))
            linC.args.push_back(tree)

        # Setting the type of our lin op
        linC.type = get_type(linPy.type.upper())

        # Setting size
        linC.size.push_back(int(linPy.size[0]))
        linC.size.push_back(int(linPy.size[1]))

        # Loading the problem data into the appropriate array format
        if linPy.data is None:
            pass
        elif isinstance(linPy.data, tuple) and isinstance(linPy.data[0], slice):
            set_slice_data(linC, linPy)
        elif isinstance(linPy.data, float) or isinstance(linPy.data, int):
            linC.set_dense_data(format_matrix(linPy.data, 'scalar'))
        elif isinstance(linPy.data, LinOp) and linPy.data.type == 'scalar_const':
            linC.set_dense_data(format_matrix(linPy.data.data, 'scalar'))
        else:
            set_matrix_data(linC, linPy)

    return root_linC


def get_constraint_node(c, tmp):
    root = CVXcanon.LinOp()
    root.size.push_back(c.size[0])
    root.size.push_back(c.size[1])

    # add ID as dense_data
    root.set_dense_data(format_matrix(c.constr_id, 'scalar'))

    if isinstance(c, LinEqConstr):
        root.type = CVXcanon.EQ
        expr = build_lin_op_tree(c.expr, tmp)
        tmp.append(expr)
        root.args.push_back(expr)

    elif isinstance(c, LinLeqConstr):
        root.type = CVXcanon.LEQ
        expr = build_lin_op_tree(c.expr, tmp)
        tmp.append(expr)
        root.args.push_back(expr)

    elif isinstance(c, SOC):
        root.type = CVXcanon.SOC
        t = build_lin_op_tree(c.t, tmp)
        tmp.append(t)
        root.args.push_back(t)
        for elem in c.x_elems:
            x_elem = build_lin_op_tree(elem, tmp)
            tmp.append(x_elem)
            root.args.push_back(x_elem)

    elif isinstance(c, ExpCone):
        root.type = CVXcanon.EXP
        x = build_lin_op_tree(c.x, tmp)
        y = build_lin_op_tree(c.y, tmp)
        z = build_lin_op_tree(c.z, tmp)
        root.args.push_back(x)
        root.args.push_back(y)
        root.args.push_back(z)
        tmp += [x, y, z]

    elif isinstance(c, SDP):
        raise NotImplementedError("SDP")

    else:
        raise TypeError("Undefined constraint type")

    return root


## new interface
def solve(sense, objective, constraints, verbose, solver_options):
    # This array keeps variables data in scope
    # after build_lin_op_tree returns
    tmp = []

    C_objective = build_lin_op_tree(objective, tmp)

    C_constraints = CVXcanon.LinOpVector()
    for constr in constraints:
        root = get_constraint_node(constr, tmp)
        tmp.append(root)
        C_constraints.push_back(root)

    if sense == Minimize:
        C_sense = CVXcanon.MINIMIZE
    elif sense == Maximize:
        C_sense = CVXcanon.MAXIMIZE
    else:
        raise NotImplementedError()

    C_opts = CVXcanon.StringDoubleMap(solver_options)
    C_opts['verbose'] = float(verbose)

    solution = CVXcanon.solve(C_sense, C_objective, C_constraints, C_opts)

    print 'CVXcanon optimal value: ', solution.optimal_value

    return solution
