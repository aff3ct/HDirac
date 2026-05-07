#include <sstream>

#include "Module/Cross_rotation_computer.hpp"
#include "Tools/Helper.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Cross_rotation_computer<T>::Cross_rotation_computer(const size_t n,
                                                    const size_t d,
                                                    const size_t sub_vec_length,
                                                    aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
  , normal_gen(normal_gen.clone())
{
    const std::string name = "Cross_rotation_computer";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d != 64)
    {
        std::stringstream message;
        message << "'d' should be equal to 64 ('d' = " << d << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->u_tilde = new double[this->n];
    this->y_tilde = new double[this->n];
    this->temp_y = new double[8];
    this->temp_y_tilde = new double[8];
    this->temp_u_tilde = new double[8];
    this->temp_u = new double[8];

    this->x_tilde = new double[this->n];
    this->temp_x_tilde = new double[8];
    this->temp_x = new double[8];

    auto& p1 = this->create_task("compute_cross_rot");
    auto p1s_in_U = this->template create_socket_in<float>(p1, "in_U", this->n);
    auto p1s_in_Y = this->template create_socket_in<T>(p1, "in_Y", this->n);
    auto p1s_out_Y_frob_norms = this->template create_socket_out<T>(p1, "out_Y_frob_norms", this->sub_vec_length);
    auto p1s_out_cross_rotations = this->template create_socket_out<T>(p1, "out_cross_rotations", this->n * 2);
    this->create_codelet(p1,
                         [p1s_in_U, p1s_in_Y, p1s_out_Y_frob_norms, p1s_out_cross_rotations](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& cr_rot_cmp = static_cast<Cross_rotation_computer&>(m);
                             cr_rot_cmp._compute_cross_rot(t[p1s_in_U].template get_dataptr<const float>(),
                                                           t[p1s_in_Y].template get_dataptr<const T>(),
                                                           t[p1s_out_Y_frob_norms].template get_dataptr<T>(),
                                                           t[p1s_out_cross_rotations].template get_dataptr<T>(),
                                                           frame_id);
                             return runtime::status_t::SUCCESS;
                         });

    auto& p2 = this->create_task("compute_reverse_cross_rot");
    auto p2s_in_cross_rotations = this->template create_socket_in<T>(p2, "in_cross_rotations", this->n * 2);
    auto p2s_in_X = this->template create_socket_in<T>(p2, "in_X", this->n);
    auto p2s_out_reverse_cross_rotations =
      this->template create_socket_out<T>(p2, "out_reverse_cross_rotations", this->n);
    this->create_codelet(p2,
                         [p2s_in_cross_rotations, p2s_in_X, p2s_out_reverse_cross_rotations](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& cr_rev_rot_cmp = static_cast<Cross_rotation_computer&>(m);
                             cr_rev_rot_cmp._compute_reverse_cross_rot(
                               t[p2s_in_cross_rotations].template get_dataptr<const T>(),
                               t[p2s_in_X].template get_dataptr<const T>(),
                               t[p2s_out_reverse_cross_rotations].template get_dataptr<T>(),
                               frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Cross_rotation_computer<T>::~Cross_rotation_computer()
{
    delete[] u_tilde;
    delete[] y_tilde;
    delete[] temp_y;
    delete[] temp_y_tilde;
    delete[] temp_u_tilde;
    delete[] temp_u;

    delete[] x_tilde;
    delete[] temp_x_tilde;
    delete[] temp_x;
}

template<typename T>
Cross_rotation_computer<T>*
Cross_rotation_computer<T>::clone() const
{
    auto m = new Cross_rotation_computer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
Cross_rotation_computer<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Cross_rotation_computer<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
Cross_rotation_computer<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
void
Cross_rotation_computer<T>::set_seed(const int seed)
{
    this->normal_gen->set_seed(seed);
}

template<typename T>
void
Cross_rotation_computer<T>::deep_copy(const Cross_rotation_computer<T>& m)
{
    spu::module::Stateful::deep_copy(m);

    this->u_tilde = new double[this->n];
    this->y_tilde = new double[this->n];
    this->temp_y = new double[8];
    this->temp_y_tilde = new double[8];
    this->temp_u_tilde = new double[8];
    this->temp_u = new double[8];

    this->x_tilde = new double[this->n];
    this->temp_x_tilde = new double[8];
    this->temp_x = new double[8];

    if (m.u_tilde != nullptr)
    {
        std::copy(m.u_tilde, m.u_tilde + this->n, this->u_tilde);
    }

    if (m.y_tilde != nullptr)
    {
        std::copy(m.y_tilde, m.y_tilde + this->n, this->y_tilde);
    }

    if (m.temp_y != nullptr)
    {
        std::copy(m.temp_y, m.temp_y + 8, this->temp_y);
    }

    if (m.temp_y_tilde != nullptr)
    {
        std::copy(m.temp_y_tilde, m.temp_y_tilde + 8, this->temp_y_tilde);
    }

    if (m.temp_u_tilde != nullptr)
    {
        std::copy(m.temp_u_tilde, m.temp_u_tilde + 8, this->temp_u_tilde);
    }

    if (m.temp_u != nullptr)
    {
        std::copy(m.temp_u, m.temp_u + 8, this->temp_u);
    }

    if (m.x_tilde != nullptr)
    {
        std::copy(m.x_tilde, m.x_tilde + this->n, this->x_tilde);
    }

    if (m.temp_x_tilde != nullptr)
    {
        std::copy(m.temp_x_tilde, m.temp_x_tilde + 8, this->temp_x_tilde);
    }

    if (m.temp_x != nullptr)
    {
        std::copy(m.temp_x, m.temp_x + 8, this->temp_x);
    }

    if (m.normal_gen != nullptr)
    {
        this->normal_gen.reset(m.normal_gen->clone());
    }
}

template<typename T>
void
Cross_rotation_computer<T>::compute_cross_rot(const float* in_U,
                                              const T* in_Y,
                                              T* out_Y_frob_norms,
                                              T* out_cross_rotations,
                                              const int frame_id,
                                              const bool managed_memory)
{
    (*this)[cross_rot_cmp::sck::compute_cross_rot::in_U].bind(in_U);
    (*this)[cross_rot_cmp::sck::compute_cross_rot::in_Y].bind(in_Y);
    (*this)[cross_rot_cmp::sck::compute_cross_rot::out_Y_frob_norms].bind(out_Y_frob_norms);
    (*this)[cross_rot_cmp::sck::compute_cross_rot::out_cross_rotations].bind(out_cross_rotations);
    (*this)[cross_rot_cmp::tsk::compute_cross_rot].exec(frame_id, managed_memory);
}

template<typename T>
void
Cross_rotation_computer<T>::compute_reverse_cross_rot(const T* in_cross_rotations,
                                                      const T* in_X,
                                                      T* out_reverse_cross_rotations,
                                                      const int frame_id,
                                                      const bool managed_memory)
{
    (*this)[cross_rot_cmp::sck::compute_reverse_cross_rot::in_cross_rotations].bind(in_cross_rotations);
    (*this)[cross_rot_cmp::sck::compute_reverse_cross_rot::in_X].bind(in_X);
    (*this)[cross_rot_cmp::sck::compute_reverse_cross_rot::out_reverse_cross_rotations].bind(
      out_reverse_cross_rotations);
    (*this)[cross_rot_cmp::tsk::compute_reverse_cross_rot].exec(frame_id, managed_memory);
}

template<typename T>
void
Cross_rotation_computer<T>::_compute_cross_rot(const float* in_U,
                                               const T* in_Y,
                                               T* out_Y_frob_norms,
                                               T* out_cross_rotations,
                                               const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        // Frobenius norm computation on new formed "matrix" of size 64
        out_Y_frob_norms[i] = 0.0;
        for (size_t j = 0; j < this->d; j++)
        {
            out_Y_frob_norms[i] += in_Y[i * this->d + j] * in_Y[i * this->d + j];
        }
        out_Y_frob_norms[i] = std::sqrt(out_Y_frob_norms[i]);

        // Generation of a random matrix with i.i.d. standard normal entries
        this->normal_gen->generate(&this->u_tilde[i * this->d], this->d, 1.0, 0.0);

        // Column transformation
        for (size_t j = 0; j < 8; j++)
        {
            // Compute column norm of y and u_tilde
            double norm_y_col = 0.0;
            double norm_u_col = 0.0;
            for (size_t k = 0; k < 8; k++)
            {
                norm_y_col += in_Y[i * this->d + j + k * 8] * in_Y[i * this->d + j + k * 8];
                norm_u_col += this->u_tilde[i * this->d + j + k * 8] * this->u_tilde[i * this->d + j + k * 8];
            }
            norm_y_col = std::sqrt(norm_y_col);
            norm_u_col = std::sqrt(norm_u_col);

            // Normalise and construct temporary vectors (y and u_tilde) from matrix column
            for (size_t k = 0; k < 8; k++)
            {
                this->temp_y[k] = in_Y[i * this->d + j + k * 8] / norm_y_col;
                this->temp_u_tilde[k] = u_tilde[i * this->d + j + k * 8] / norm_u_col;
            }

            // Compute column transformation
            CVQKD::tools::octonion_division(
              this->temp_u_tilde, this->temp_y, &out_cross_rotations[i * this->d + j * 8]);
            // Compute y_tilde matrix column
            CVQKD::tools::octonion_multiplication(
              &out_cross_rotations[i * this->d + j * 8], this->temp_y, this->temp_y_tilde);

            // Put back into y_tilde column into y_tilde matrix
            for (size_t k = 0; k < 8; k++)
            {
                this->y_tilde[i * this->d + j + k * 8] = this->temp_y_tilde[k];
            }
        }

        // Row transformation
        for (size_t j = 0; j < 8; j++)
        {
            // Compute row norm of y_tilde and u
            double norm_y_row = 0.0;
            double norm_u_row = 0.0;
            for (size_t k = 0; k < 8; k++)
            {
                norm_y_row += this->y_tilde[i * this->d + j * 8 + k] * this->y_tilde[i * this->d + j * 8 + k];
                norm_u_row += in_U[i * this->d + j * 8 + k] * in_U[i * this->d + j * 8 + k];
            }
            norm_y_row = std::sqrt(norm_y_row);
            norm_u_row = std::sqrt(norm_u_row);

            // Normalise and construct temporary vectors (y_tilde and u) from matrix row
            for (size_t k = 0; k < 8; k++)
            {
                this->temp_y_tilde[k] = this->y_tilde[i * this->d + j * 8 + k] / norm_y_row;
                this->temp_u[k] = in_U[i * this->d + j * 8 + k] / norm_u_row;
            }

            // Compute row transformation
            CVQKD::tools::octonion_division(
              this->temp_u, this->temp_y_tilde, &out_cross_rotations[this->n + (i * this->d + j * 8)]);
        }
    }
}

template<typename T>
void
Cross_rotation_computer<T>::_compute_reverse_cross_rot(const T* in_cross_rotations,
                                                       const T* in_X,
                                                       T* out_reverse_cross_rotations,
                                                       const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            // Compute column norm of x
            double norm_x_col = 0.0;
            for (size_t k = 0; k < 8; k++)
            {
                norm_x_col += in_X[i * this->d + j + k * 8] * in_X[i * this->d + j + k * 8];
            }
            norm_x_col = std::sqrt(norm_x_col);

            // Normalise and construct temporary vector (x) from matrix column
            for (size_t k = 0; k < 8; k++)
            {
                this->temp_x[k] = in_X[i * this->d + j + k * 8] / norm_x_col;
            }

            // Compute reverse column transformation
            CVQKD::tools::octonion_multiplication(
              &in_cross_rotations[i * this->d + j * 8], this->temp_x, this->temp_x_tilde);

            // Put back into x_tilde column into x_tilde matrix
            for (size_t k = 0; k < 8; k++)
            {
                this->x_tilde[i * this->d + j + k * 8] = this->temp_x_tilde[k];
            }
        }

        for (size_t j = 0; j < 8; j++)
        {
            // Compute row norm of x_tilde
            double norm_x_row = 0.0;
            for (size_t k = 0; k < 8; k++)
            {
                norm_x_row += this->x_tilde[i * this->d + j * 8 + k] * this->x_tilde[i * this->d + j * 8 + k];
            }
            norm_x_row = std::sqrt(norm_x_row);

            // Normalise and construct temporary vector (x_tilde) from matrix row
            for (size_t k = 0; k < 8; k++)
            {
                this->temp_x[k] = this->x_tilde[i * this->d + j * 8 + k] / norm_x_row;
            }

            // Compute - final - reverse row transformation
            CVQKD::tools::octonion_multiplication(&in_cross_rotations[this->n + (i * this->d + j * 8)],
                                                  this->temp_x,
                                                  &out_reverse_cross_rotations[i * this->d + j * 8]);
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Cross_rotation_computer<double>;
// ==================================================================================== explicit template instantiation
