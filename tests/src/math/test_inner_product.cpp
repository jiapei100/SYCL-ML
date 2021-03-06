#include <iostream>

#include "ml/math/vec_ops.hpp"
#include "utils/utils.hpp"

template <class T>
void test_inner_product_self() {
  constexpr size_t SIZE = 4;
  std::array<T, SIZE> in {1, 0.5, -1, 0};
  T res;

  {
    cl::sycl::queue& q = create_queue();
    ml::vector_t<T> sycl_vec(in.data(), cl::sycl::range<1>(in.size()));
    res = ml::sycl_inner_product(q, sycl_vec);
    clear_eigen_device();
  }

  for (unsigned i = 0; i < SIZE; ++i)
    std::cout << in[i] << " ";
  std::cout << "\nres=" << res << std::endl;

  assert_almost_eq(res, T(2.25));
}

template <class T>
void test_inner_product_other() {
  constexpr size_t SIZE = 4;
  std::array<T, SIZE> in1 {1, 2, 3, 4};
  std::array<T, SIZE> in2 {2, 2, 1, 0.5};
  T res;

  {
    cl::sycl::queue& q = create_queue();
    ml::vector_t<T> sycl_vec1(in1.data(), cl::sycl::range<1>(in1.size()));
    sycl_vec1.set_final_data(nullptr);
    ml::vector_t<T> sycl_vec2(in2.data(), cl::sycl::range<1>(in2.size()));
    sycl_vec2.set_final_data(nullptr);
    res = ml::sycl_inner_product(q, sycl_vec1, sycl_vec2);
    clear_eigen_device();
  }

  for (unsigned i = 0; i < SIZE; ++i)
    std::cout << in1[i] << " ";
  std::cout << std::endl;
  for (unsigned i = 0; i < SIZE; ++i)
    std::cout << in2[i] << " ";
  std::cout << "\nres=" << res << std::endl;

  assert_almost_eq(res, T(11));
}

int main() {
  try {
    test_inner_product_self<ml::buffer_data_type>();
    test_inner_product_other<ml::buffer_data_type>();
  } catch (cl::sycl::exception e) {
    std::cerr << e.what();
  }

  return 0;
}
