#ifndef HELPER_HPP_
#define HELPER_HPP_

#include <iostream>

namespace CVQKD
{
namespace tools
{
float
beta_to_snr(const float beta, const float code_rate);

float
beta_to_snr(const float beta, const float code_rate, const bool binary_channel);

float
beta_to_snr_BIAWGN(const float beta, const float code_rate);

double
compute_norm(const double* vector, const size_t vec_size);

double
compute_norm2(const double* vec, const size_t vec_size);

double
dot_product(const double* u, const double* v, const size_t vec_size);

void
complex_multiplication(const float* in_a, const double* in_b, double* out_complex_product);

void
complex_division(const double* in_a, const double* in_b, double* out_complex_quotient);

void
quaternion_conjugate(const double* q, double* out);

void
quaternion_multiplication(const double* in_a, const double* in_b, double* out_quaternion_product);

void
quaternion_multiplication(const float* in_a, const double* in_b, double* out_quaternion_product);

void
quaternion_division(const double* in_a, const double* in_b, double* out_quaternion_quotient);

void
octonion_multiplication(const float* in_a, const double* in_b, double* out_octonion_product);

void
octonion_multiplication(const double* in_a, const double* in_b, double* out_octonion_product);

void
octonion_division(const double* in_a, const double* in_b, double* out_octonion_quotient);

void
octonion_division(const float* in_a, const double* in_b, double* out_octonion_quotient);

}
}

#endif
