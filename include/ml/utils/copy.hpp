#ifndef INCLUDE_ML_UTILS_COPY_HPP
#define INCLUDE_ML_UTILS_COPY_HPP

#include "ml/utils/sycl_types.hpp"
#include "ml/utils/sycl_helper.hpp"
#include "ml/utils/debug/assert.hpp"

namespace ml
{

class ml_memset;

/**
 * @brief Memset a specific range to a SYCL buffer.
 */
template <class T, int DIM>
void sycl_memset(queue& q, buffer<T, DIM>& buffer, const nd_range<DIM>& r, T val = T(0)) {
  assert_less_or_eq(r.get_offset()[0] + r.get_global_linear_range(), buffer.get_count());
  q.submit([&](handler& cgh) {
    auto acc = buffer.template get_access<access::mode::write>(cgh);
    cgh.parallel_for<NameGen<DIM, ml_memset, T>>(r, [=](nd_item<DIM> item) {
      acc[item.get_global_linear_id()] = val;
    });
  });
}

/**
 * @brief Memset a whole SYCL buffer.
 */
template <class T, int DIM>
void sycl_memset(queue& q, buffer<T, DIM>& buffer, T val = T(0)) {
  q.submit([&](handler& cgh) {
    auto acc = buffer.template get_access<access::mode::discard_write>(cgh);
    cgh.fill(acc, val);
  });
}

/**
 * @brief Copy \p src to \p dst.
 */
template <class T, int DIM>
void sycl_copy(queue& q, buffer<T, DIM>& src, buffer<T, DIM>& dst) {
  assert_eq(src.get_count(), dst.get_count());

  q.submit([&](handler& cgh) {
    auto src_acc = src.template get_access<access::mode::read>(cgh);
    auto dst_acc = dst.template get_access<access::mode::discard_write>(cgh);
    cgh.copy(src_acc, dst_acc);
  });
}

/**
 * @brief Copy a host buffer to device.
 */
template <class T, class SrcPtrT>
void sycl_copy_host_to_device(queue& q, SrcPtrT src, sycl_vec_t<T>& dst) {
  q.submit([&](handler& cgh) {
    auto dst_acc = dst.template get_access<access::mode::discard_write>(cgh);
    cgh.copy(src, dst_acc);
  });
}

/**
 * @brief Copy a device buffer to host.
 */
template <class T, class DstPtrT>
void sycl_copy_device_to_host(queue& q, sycl_vec_t<T>& src, DstPtrT dst) {
  q.submit([&](handler& cgh) {
    auto src_acc = src.template get_access<access::mode::read>(cgh);
    cgh.copy(src_acc, dst);
  });
}

class ml_init_func_i;

/**
 * @brief Init a SYCL buffer with a function depending on the index.
 */
template <class Op, class T, int DIM>
void sycl_init_func_i(queue& q, buffer<T, DIM>& buffer, const nd_range<DIM>& r, Op op = Op()) {
  assert_less_or_eq(r.get_offset()[0] + r.get_global_linear_range(), buffer.get_count());
  q.submit([&](handler& cgh) {
    auto acc = buffer.template get_access<access::mode::discard_write>(cgh);
    cgh.parallel_for<NameGen<DIM, ml_init_func_i, T, Op>>(r, [=](nd_item<DIM> item) {
      auto idx = item.get_global_linear_id();
      acc[idx] = op(idx);
    });
  });
}

} // ml

#endif //INCLUDE_ML_UTILS_COPY_HPP
