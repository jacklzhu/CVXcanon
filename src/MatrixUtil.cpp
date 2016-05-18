
#include "MatrixUtil.hpp"

void append_block_triplets(
    const SparseMatrix& A, int i, int j,
    std::vector<Triplet>* coeffs) {
  for (int k = 0; k < A.outerSize(); k++) {
    for (SparseMatrix::InnerIterator iter(A, k); iter; ++iter) {
      coeffs->push_back(Triplet(i+iter.row(), j+iter.col(), iter.value()));
    }
  }
}

SparseMatrix sparse_matrix(int m, int n, const std::vector<Triplet>& coeffs) {
  SparseMatrix A(m, n);
  A.setFromTriplets(coeffs.begin(), coeffs.end());
  return A;
}

SparseMatrix scalar_matrix(double alpha, int n) {
  return (alpha*DenseMatrix::Identity(n,n)).sparseView();
}

SparseMatrix ones_matrix(int m, int n) {
  return DenseMatrix::Constant(m, n, 1).sparseView();
}

SparseMatrix identity(int n) {
  return DenseMatrix::Identity(n,n).sparseView();
}

SparseMatrix reshape(const SparseMatrix& A, int m, int n) {
  assert(A.rows()*A.cols() == m*n);
  // TODO(mwytock): More efficient way?
  DenseMatrix A_copy = A;
  return Eigen::Map<const DenseMatrix>(A_copy.data(), m, n).sparseView();
}

DenseVector to_vector(const DenseMatrix& A) {
  return Eigen::Map<const DenseVector>(A.data(), A.rows()*A.cols());
}

std::string vector_debug_string(const DenseVector& x) {
  std::string retval("[");

  const int max_elems = 10;
  if (x.size() > max_elems) {
    for (int i = 0; i < max_elems/2; i++) {
      retval += string_printf("%.4f ", x[i]);
    }

    retval += "... ";

    for (int i = x.size()-max_elems/2-1; i < x.size(); i++) {
      retval += string_printf("%.4f ", x[i]);
    }
  } else {
    for (int i = 0; i < x.size(); i++) {
      retval += string_printf("%.4f ", x[i]);
    }
  }

  if (retval.size() > 1) {
    retval = retval.substr(0, retval.size()-1) + "]";
  } else {
    retval += "]";
  }

  return retval;
}

std::string matrix_debug_string(const SparseMatrix& A) {
  std::string retval;

  const int max_rows = 10;
  if (A.rows() > max_rows) {
    for (int i = 0; i < max_rows/2; i++) {
      retval += vector_debug_string(A.row(i)) + "\n";
    }
    retval += "...\n";
    for (int i = A.rows()-max_rows/2-1; i < A.rows(); i++) {
      retval += vector_debug_string(A.row(i)) + "\n";
    }
  } else {
    for (int i = 0; i < A.rows(); i++) {
      retval += vector_debug_string(A.row(i)) + "\n";
    }
  }

  return retval;
}
