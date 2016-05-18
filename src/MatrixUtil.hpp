
#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include "Utils.hpp"

void append_block_triplets(
    const SparseMatrix& A, int i, int j,
    std::vector<Triplet>* coeffs);

SparseMatrix sparse_matrix(int m, int n, const std::vector<Triplet>& coeffs);
SparseMatrix scalar_matrix(double alpha, int n);
SparseMatrix ones_matrix(int m, int n);
SparseMatrix identity(int n);
SparseMatrix reshape(const SparseMatrix& A, int m, int n);

DenseVector to_vector(const DenseMatrix& A);

std::string vector_debug_string(const DenseVector& a);

template<typename MatrixType>
std::string matrix_debug_string(const MatrixType& A) {
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


#endif // MATRIX_UTIL_H
