#include "Tools/Helper.hpp"

#include <cmath>
#define _USE_MATH_DEFINES

/*
 * Capacity of an AWGNC
 * Beta expressed as R / (0.5 * log2(1 + SNR))
 */
float
CVQKD::tools::beta_to_snr(const float beta, const float code_rate)
{
    return std::pow(2, (2 * code_rate / beta)) - 1;
}

float
CVQKD::tools::beta_to_snr(const float beta, const float code_rate, const bool binary_channel)
{
    if (binary_channel)
    {
        return beta_to_snr_BIAWGN(beta, code_rate);
    }

    return beta_to_snr(beta, code_rate);
}

/*
 * PDF of a BIAWGNC with a BPSK modulation
 */
float
phi(float x, float a)
{
    float term1 = std::exp(-std::pow((x - 1), 2) / (2 * a));
    float term2 = std::exp(-std::pow((x + 1), 2) / (2 * a));
    return (1.0 / std::sqrt(8 * M_PI * a)) * (term1 + term2);
}

float
H_y_integrand(float x, float a)
{
    float p = phi(x, a);
    if (p <= 1e-14f) return 0.0f;
    return p * std::log2(p);
}

float
integrate(float a)
{
    const float L = 10.0f; // truncate range [-L, L]
    const int N = 10000;
    const float h = (2 * L) / N;

    float sum = H_y_integrand(-L, a) + H_y_integrand(L, a);

    for (int i = 1; i < N; ++i)
    {
        float x = -L + i * h;
        if (i % 2 == 0)
        {
            sum += 2 * H_y_integrand(x, a);
        }
        else
        {
            sum += 4 * H_y_integrand(x, a);
        }
    }

    return (h / 3.0f) * sum;
}

/*
 * Computing mutual information (capacity) for a BIAWGNC; I(X;Y) = H(Y|X) - H(X)
 */
float
simulated_BIAWGNC_capacity(const float a)
{
    float H_Y_integral = integrate(a);
    float H_Y = -H_Y_integral;

    float H_Y_given_X = 0.5f * std::log2(2 * M_PI * M_E * a);

    return H_Y - H_Y_given_X;
}

/*
 * Approximated capacity for a BIAWGNC
 */
float
CVQKD::tools::beta_to_snr_BIAWGN(const float beta, const float code_rate)
{
    const float LOWER_THRESHOLD = 0.425f;   // Max. capacity for which we can make an approximation using the AWGNC
    const float HIGHER_THRESHOLD = 0.9965f; // Capacity approximation threshold for 1 bit/symb

    float capacity = code_rate / beta;

    // The capacity of the BIAWGNC can be approximated by that of the AWGNC
    // SNR is then computed as if it was an AWGNC
    if (capacity <= LOWER_THRESHOLD)
    {
        return beta_to_snr(beta, code_rate);
    }

    // Capacity is considered equal to one above that threshold for any further SNR values
    if (capacity >= HIGHER_THRESHOLD)
    {
        // Signal variance is 1.0 since symbols are uniformly distributed over {-1, +1}
        return 1.0f / (code_rate * beta);
    }

    const float EPSILON = 1e-4f;
    const size_t MAX_ITER = 20;
    float lower_variance = 0.1f;   // capacity is almost 1
    float higher_variance = 1.25f; // capacity is almost the same as a AWGNC
    float variance = (higher_variance + lower_variance) / 2.0f;
    float simulated_capacity = simulated_BIAWGNC_capacity(variance);

    size_t cnt = 0;

    // Dichotomy variance search for wich the simulated BIAWGNC capacity is close to the actual capacity
    while (std::abs(simulated_capacity - capacity) > EPSILON && lower_variance != higher_variance && cnt < MAX_ITER)
    {
        if (simulated_capacity > capacity)
        {
            lower_variance = variance;
        }
        else if (simulated_capacity < capacity)
        {
            higher_variance = variance;
        }
        else
        {
            break;
        }
        variance = (higher_variance + lower_variance) / 2;
        simulated_capacity = simulated_BIAWGNC_capacity(variance);
        cnt++;
    }

    const float noise_variance = variance;

    // Signal variance is 1.0 since symbols are uniformly distributed over {-1, +1}
    return 1.0f / noise_variance;
}

double
CVQKD::tools::compute_norm(const double* vector, const size_t vec_size)
{
    return std::sqrt(compute_norm2(vector, vec_size));
}

double
CVQKD::tools::compute_norm2(const double* vec, const size_t vec_size)
{
    double norm2 = 0.0;

    for (size_t i = 0; i < vec_size; i++)
    {
        norm2 += (vec[i] * vec[i]);
    }

    return norm2;
}

double
CVQKD::tools::dot_product(const double* u, const double* v, const size_t vec_size)
{
    double dot_product = 0.0;

    for (size_t i = 0; i < vec_size; i++)
    {
        dot_product += u[i] * v[i];
    }

    return dot_product;
}

void
CVQKD::tools::complex_multiplication(const float* in_a, const double* in_b, double* out_complex_product)
{
    out_complex_product[0] = (in_a[0] * in_b[0]) - (in_a[1] * in_b[1]);
    out_complex_product[1] = (in_a[0] * in_b[1]) + (in_a[1] * in_b[0]);
}

void
CVQKD::tools::complex_division(const double* in_a, const double* in_b, double* out_complex_quotient)
{
    double div = (in_b[0] * in_b[0]) + (in_b[1] * in_b[1]);
    out_complex_quotient[0] = (in_a[0] * in_b[0]) + (in_a[1] * in_b[1]);
    out_complex_quotient[0] /= div;
    out_complex_quotient[1] = (in_a[1] * in_b[0]) - (in_a[0] * in_b[1]);
    out_complex_quotient[1] /= div;
}

void
CVQKD::tools::quaternion_conjugate(const double* q, double* out)
{
    out[0] = q[0];
    out[1] = -q[1];
    out[2] = -q[2];
    out[3] = -q[3];
}

void
CVQKD::tools::quaternion_multiplication(const double* in_a, const double* in_b, double* out_quaternion_product)
{
    const double aw = in_a[0];
    const double ax = in_a[1];
    const double ay = in_a[2];
    const double az = in_a[3];

    const double bw = in_b[0];
    const double bx = in_b[1];
    const double by = in_b[2];
    const double bz = in_b[3];

    out_quaternion_product[0] = aw * bw - ax * bx - ay * by - az * bz;
    out_quaternion_product[1] = aw * bx + ax * bw + ay * bz - az * by;
    out_quaternion_product[2] = aw * by - ax * bz + ay * bw + az * bx;
    out_quaternion_product[3] = aw * bz + ax * by - ay * bx + az * bw;
}

void
CVQKD::tools::quaternion_multiplication(const float* in_a, const double* in_b, double* out_quaternion_product)
{
    double in_a_double[4];
    for (size_t i = 0; i < 4; i++)
    {
        in_a_double[i] = static_cast<double>(in_a[i]);
    }
    quaternion_multiplication(in_a_double, in_b, out_quaternion_product);
}

void
CVQKD::tools::quaternion_division(const double* in_a, const double* in_b, double* out_quaternion_quotient)
{
    // inverse(b) = conjugate(b) / |b|^2
    const double bw = in_b[0];
    const double bx = in_b[1];
    const double by = in_b[2];
    const double bz = in_b[3];

    const double norm2 = bw * bw + bx * bx + by * by + bz * bz;

    const double inv_bw = bw / norm2;
    const double inv_bx = -bx / norm2;
    const double inv_by = -by / norm2;
    const double inv_bz = -bz / norm2;

    // out = a * inverse(b)
    const double aw = in_a[0];
    const double ax = in_a[1];
    const double ay = in_a[2];
    const double az = in_a[3];

    out_quaternion_quotient[0] = aw * inv_bw - ax * inv_bx - ay * inv_by - az * inv_bz;
    out_quaternion_quotient[1] = aw * inv_bx + ax * inv_bw + ay * inv_bz - az * inv_by;
    out_quaternion_quotient[2] = aw * inv_by - ax * inv_bz + ay * inv_bw + az * inv_bx;
    out_quaternion_quotient[3] = aw * inv_bz + ax * inv_by - ay * inv_bx + az * inv_bw;
}

void
CVQKD::tools::octonion_multiplication(const float* in_a, const double* in_b, double* out_octonion_product)
{
    double a[8];
    for (int i = 0; i < 8; i++)
        a[i] = static_cast<double>(in_a[i]);

    CVQKD::tools::octonion_multiplication(a, in_b, out_octonion_product);
}

void
CVQKD::tools::octonion_multiplication(const double* in_a, const double* in_b, double* out_octonion_product)
{
    // Split into quaternions
    const double* a0 = &in_a[0];
    const double* a1 = &in_a[4];

    const double* b0 = &in_b[0];
    const double* b1 = &in_b[4];

    double b0_conj[4], b1_conj[4];
    quaternion_conjugate(b0, b0_conj);
    quaternion_conjugate(b1, b1_conj);

    double t0[4], t1[4], t2[4], t3[4];

    // out0 = a0*b0 - conj(b1)*a1
    quaternion_multiplication(a0, b0, t0);
    quaternion_multiplication(b1_conj, a1, t1);

    out_octonion_product[0] = t0[0] - t1[0];
    out_octonion_product[1] = t0[1] - t1[1];
    out_octonion_product[2] = t0[2] - t1[2];
    out_octonion_product[3] = t0[3] - t1[3];

    // out1 = b1*a0 + a1*conj(b0)
    quaternion_multiplication(b1, a0, t2);
    quaternion_multiplication(a1, b0_conj, t3);

    out_octonion_product[4] = t2[0] + t3[0];
    out_octonion_product[5] = t2[1] + t3[1];
    out_octonion_product[6] = t2[2] + t3[2];
    out_octonion_product[7] = t2[3] + t3[3];
}

void
CVQKD::tools::octonion_division(const double* in_a, const double* in_b, double* out_octonion_quotient)
{
    // |b|^2
    double norm2 = 0.0;
    for (int i = 0; i < 8; i++)
        norm2 += in_b[i] * in_b[i];

    // conjugate(b)
    double b_conj[8] = { in_b[0], -in_b[1], -in_b[2], -in_b[3], -in_b[4], -in_b[5], -in_b[6], -in_b[7] };

    // a * conjugate(b)
    double tmp[8];

    octonion_multiplication(in_a, b_conj, tmp);

    for (int i = 0; i < 8; i++)
        out_octonion_quotient[i] = tmp[i] / norm2;
}

void
CVQKD::tools::octonion_division(const float* in_a, const double* in_b, double* out_octonion_quotient)
{
    double a_double[8];
    for (int i = 0; i < 8; i++)
        a_double[i] = static_cast<double>(in_a[i]);

    CVQKD::tools::octonion_division(a_double, in_b, out_octonion_quotient);
}
