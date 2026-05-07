#include <sstream>

#include "Module/Rotation_vectors_computer.hpp"
#include "Tools/Helper.hpp"

#define EPSILON 1e-12

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Rotation_vectors_computer<T>::Rotation_vectors_computer(const size_t n,
                                                        const size_t d,
                                                        const size_t sub_vec_length,
                                                        aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
  , vectors_array_size((d * (d + 1) / 2) * sub_vec_length)
  , normal_gen(normal_gen.clone())
{
    const std::string name = "Rotation_vectors_computer";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d == 0)
    {
        std::stringstream message;
        message << "'d' has to be greater than 0 ('d' = " << d << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->scaled_u = new double[this->n];
    this->random_vectors = new double[this->vectors_array_size];
    this->g_temp = new double[this->vectors_array_size];
    this->u_temp = new double[this->vectors_array_size];

    auto& p1 = this->create_task("compute_rotations");
    auto p1s_in_norms2 = this->template create_socket_in<T>(p1, "in_norms2", this->sub_vec_length);
    auto p1s_in_Y = this->template create_socket_in<T>(p1, "in_Y", this->n);
    auto p1s_in_U = this->template create_socket_in<float>(p1, "in_U", this->n);
    auto p1s_out_rotation_vectors =
      this->template create_socket_out<T>(p1, "out_rotation_vectors", this->vectors_array_size);
    this->create_codelet(p1,
                         [p1s_in_norms2, p1s_in_Y, p1s_in_U, p1s_out_rotation_vectors](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& rota_vec_cmp = static_cast<Rotation_vectors_computer&>(m);
                             rota_vec_cmp._compute_rotations(t[p1s_in_norms2].template get_dataptr<const T>(),
                                                             t[p1s_in_Y].template get_dataptr<const T>(),
                                                             t[p1s_in_U].template get_dataptr<const float>(),
                                                             t[p1s_out_rotation_vectors].template get_dataptr<T>(),
                                                             frame_id);
                             return runtime::status_t::SUCCESS;
                         });

    auto& p2 = this->create_task("compute_reverse_rotations");
    auto p2s_in_rotation_vectors =
      this->template create_socket_in<T>(p2, "in_rotation_vectors", this->vectors_array_size);
    auto p2s_in_X = this->template create_socket_in<T>(p2, "in_X", this->n);
    auto p2s_out_rotated_X = this->template create_socket_out<T>(p2, "out_rotated_X", this->n);
    this->create_codelet(
      p2,
      [p2s_in_rotation_vectors, p2s_in_X, p2s_out_rotated_X](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& rota_vec_cmp = static_cast<Rotation_vectors_computer&>(m);
          rota_vec_cmp._compute_reverse_rotations(t[p2s_in_rotation_vectors].template get_dataptr<const T>(),
                                                  t[p2s_in_X].template get_dataptr<const T>(),
                                                  t[p2s_out_rotated_X].template get_dataptr<T>(),
                                                  frame_id);
          return runtime::status_t::SUCCESS;
      });
}

template<typename T>
Rotation_vectors_computer<T>::~Rotation_vectors_computer()
{
    delete[] this->scaled_u;
    delete[] this->random_vectors;
    delete[] this->g_temp;
    delete[] this->u_temp;
}

template<typename T>
Rotation_vectors_computer<T>*
Rotation_vectors_computer<T>::clone() const
{
    auto m = new Rotation_vectors_computer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
void
Rotation_vectors_computer<T>::deep_copy(const Rotation_vectors_computer<T>& m)
{
    spu::module::Stateful::deep_copy(m);

    this->scaled_u = new double[this->n];
    this->random_vectors = new double[this->vectors_array_size];
    this->g_temp = new double[this->vectors_array_size];
    this->u_temp = new double[this->vectors_array_size];

    if (m.scaled_u != nullptr)
    {
        std::copy(m.scaled_u, m.scaled_u + this->n, this->scaled_u);
    }

    if (m.random_vectors != nullptr)
    {
        std::copy(m.random_vectors, m.random_vectors + this->vectors_array_size, this->random_vectors);
    }

    if (m.g_temp != nullptr)
    {
        std::copy(m.g_temp, m.g_temp + this->vectors_array_size, this->g_temp);
    }

    if (m.u_temp != nullptr)
    {
        std::copy(m.u_temp, m.u_temp + this->vectors_array_size, this->u_temp);
    }

    if (m.normal_gen != nullptr)
    {
        this->normal_gen.reset(m.normal_gen->clone());
    }
}

template<typename T>
size_t
Rotation_vectors_computer<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Rotation_vectors_computer<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
Rotation_vectors_computer<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
void
Rotation_vectors_computer<T>::set_seed(const int seed)
{
    this->normal_gen->set_seed(seed);
}

template<typename T>
void
Rotation_vectors_computer<T>::compute_rotations(const T* in_norms2,
                                                const T* in_Y,
                                                const float* in_U,
                                                T* out_rotation_vectors,
                                                const int frame_id,
                                                const bool managed_memory)
{
    (*this)[rotation_vct_cmp::sck::compute_rotations::in_norms2].bind(in_norms2);
    (*this)[rotation_vct_cmp::sck::compute_rotations::in_Y].bind(in_Y);
    (*this)[rotation_vct_cmp::sck::compute_rotations::in_U].bind(in_U);
    (*this)[rotation_vct_cmp::sck::compute_rotations::out_rotation_vectors].bind(out_rotation_vectors);
    (*this)[rotation_vct_cmp::tsk::compute_rotations].exec(frame_id, managed_memory);
}

template<typename T>
T
Rotation_vectors_computer<T>::compute_alpha(const T* u, const T* h, const T b, const size_t j)
{
    T X = CVQKD::tools::dot_product(u, h, j);
    T Y = CVQKD::tools::dot_product(u, u, j);
    T Z = CVQKD::tools::dot_product(h, h, j);

    T A = Y * Y - b * b * Y;
    T B = 2.0 * X * Y - 2.0 * b * b * X;
    T C = X * X - b * b * Z;

    if (std::abs(A) < EPSILON)
    {
        A = (A >= 0.0 ? EPSILON : -EPSILON);
    }

    T delta = B * B - 4.0 * A * C;
    if (delta < 0.0)
    {
        delta = 0.0;
    }
    T sqrt_delta = std::sqrt(delta);

    T alpha;
    // Choose roots with respect to the sign of b, since squaring the original equation ( u . (h + alpha .u) = b . || h
    // + alpha .u || ) loses the sign information
    if (b < 0.0)
    {
        alpha = (-B - sqrt_delta) / (2.0 * A);
    }
    else
    {
        alpha = (-B + sqrt_delta) / (2.0 * A);
    }

    if (std::abs(alpha) < EPSILON)
    {
        alpha = EPSILON * (alpha >= 0.0 ? 1.0 : -1.0);
    }

    return static_cast<T>(alpha);
}

template<typename T>
void
Rotation_vectors_computer<T>::_compute_rotations(const T* in_norms2,
                                                 const T* in_Y,
                                                 const float* in_U,
                                                 T* out_rotation_vectors,
                                                 const size_t /*frame_id*/)
{
    const size_t MAX_TRIES_G = 10;

    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        size_t subvector_index = i * this->d;

        // Scale u to match the norm of y ensuring that the reflection
        // hyperplane goes through the origin (required for the Householder transformation)
        T scaling_factor = std::sqrt(in_norms2[i] / this->d);

        for (size_t j = 0; j < this->d; j++)
        {
            this->scaled_u[subvector_index + j] = in_U[subvector_index + j] * scaling_factor;
        }

        // Construct reflections from dimension d to 1
        for (size_t j = this->d; j >= 1; j--)
        {
            size_t vector_first_index = (i * (this->d * (this->d + 1) / 2)) + ((j - 1) * j / 2);
            const size_t dim = j;

            // Pointer to last j coordinates of u (subpart of u)
            const T* u = &this->scaled_u[subvector_index + (this->d - dim)];

            // Pointer to h (random vector of size j)
            T* h = &this->random_vectors[vector_first_index];
            T* g = &this->g_temp[vector_first_index];

            if (dim > 1)
            {
                for (size_t l = 0; l < MAX_TRIES_G; i++)
                {
                    // Generate Gaussian vector h ~ N(0.0, 1.0)
                    this->normal_gen->generate(h, dim, 1.0, 0.0);

                    T alpha = this->compute_alpha(u, h, in_Y[subvector_index + (this->d - dim)], j);

                    // Compute g = h + alpha . u
                    for (size_t k = 0; k < dim; k++)
                    {
                        g[k] = h[k] + alpha * u[k];
                    }

                    // Normalise g (EPSILON to overcome numerical instabilities)
                    T norm_g = CVQKD::tools::compute_norm(g, dim);

                    if (norm_g > EPSILON)
                    {
                        for (size_t k = 0; k < dim; k++)
                        {
                            g[k] /= norm_g;
                        }
                    }
                    else
                    {
                        for (size_t k = 0; k < dim; k++)
                        {
                            g[k] = 0;
                        }
                    }

                    if (norm_g != 1.0 || g[0] != 1)
                    {
                        break;
                    }
                }
            }
            else
            {
                g[0] = in_Y[subvector_index + (this->d - dim)] / u[0];
            }

            // Compute Householder vector: n = (g - e1)
            g[0] -= 1;

            T* n = g;

            // Normalise g to get n (EPSILON to overcome numerical instabilities)
            T norm_n = CVQKD::tools::compute_norm(n, dim);

            if (norm_n > EPSILON)
            {
                for (size_t k = 0; k < dim; k++)
                {
                    out_rotation_vectors[vector_first_index + k] = n[k] / norm_n;
                }
            }
            else
            {
                for (size_t k = 0; k < dim; k++)
                {
                    out_rotation_vectors[vector_first_index + k] = 0;
                }
            }

            T dot_u_n = CVQKD::tools::dot_product(u, &out_rotation_vectors[vector_first_index], dim);

            // Compute Householder reflection
            for (size_t k = 0; k < dim; k++)
            {
                this->scaled_u[subvector_index + (this->d - dim) + k] =
                  u[k] - 2 * dot_u_n * out_rotation_vectors[vector_first_index + k];
            }
        }
    }
}

template<typename T>
void
Rotation_vectors_computer<T>::compute_reverse_rotations(const T* in_rotation_vectors,
                                                        const T* in_X,
                                                        T* out_rotated_X,
                                                        const int frame_id,
                                                        const bool managed_memory)
{
    (*this)[rotation_vct_cmp::sck::compute_reverse_rotations::in_rotation_vectors].bind(in_rotation_vectors);
    (*this)[rotation_vct_cmp::sck::compute_reverse_rotations::in_X].bind(in_X);
    (*this)[rotation_vct_cmp::sck::compute_reverse_rotations::out_rotated_X].bind(out_rotated_X);
    (*this)[rotation_vct_cmp::tsk::compute_reverse_rotations].exec(frame_id, managed_memory);
}

template<typename T>
void
Rotation_vectors_computer<T>::_compute_reverse_rotations(const T* in_rotation_vectors,
                                                         const T* in_X,
                                                         T* out_rotated_X,
                                                         const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        size_t subvector_index = i * this->d;

        // Copy x to output for successive composition
        for (size_t j = 0; j < this->d; j++)
        {
            out_rotated_X[subvector_index + j] = in_X[subvector_index + j];
        }

        // Apply Householder reflections from rotatons vector (normal to the hyperplane) from dimension 1 to dimension d
        for (size_t j = 1; j <= this->d; j++)
        {
            size_t vector_first_index = (i * (this->d * (this->d + 1) / 2)) + ((j - 1) * j / 2);
            const size_t dim = j;

            // Pointer to Householder vector
            const T* n = &in_rotation_vectors[vector_first_index];

            // x . n dot product
            double dot_xn = CVQKD::tools::dot_product(&out_rotated_X[subvector_index + (this->d - dim)], n, dim);

            // Compute Householder reflection and compose with the existant lower dimensions in output
            for (size_t k = 0; k < dim; k++)
            {
                out_rotated_X[subvector_index + (this->d - dim + k)] =
                  out_rotated_X[subvector_index + (this->d - dim + k)] - 2.0 * dot_xn * n[k];
            }
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Rotation_vectors_computer<double>;
// ==================================================================================== explicit template instantiation
