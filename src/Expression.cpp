
#include "Expression.hpp"
#include "MatrixUtil.hpp"

void ConstAttributes::set_dense_data(double* matrix, int rows, int cols) {
  dense_data = Eigen::Map<DenseMatrix>(matrix, rows, cols);
  // printf("set_dense_data: %d x %d\n%s",
  //        rows, cols, matrix_debug_string(dense_data).c_str());
}

Size ConstAttributes::size() const {
  //printf("size: %d x %d\n", dense_data.rows(), dense_data.cols());
  return {{static_cast<int>(dense_data.rows()),
           static_cast<int>(dense_data.cols())}};
}
