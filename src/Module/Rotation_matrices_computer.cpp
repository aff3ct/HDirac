#include <Eigen/Dense>
#include <sstream>

#include "Module/Rotation_matrices_computer.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Rotation_matrices_computer<T>::Rotation_matrices_computer(const size_t n,
                                                          const size_t d,
                                                          const size_t sub_vec_length,
                                                          aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
  , matrices_array_size(sub_vec_length * d * d)
  , normal_gen(normal_gen.clone())
{
    const std::string name = "Rotation_matrices_computer";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d == 0)
    {
        std::stringstream message;
        message << "'d' has to be greater than 0 ('d' = " << d << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->random_matrices = new double[this->matrices_array_size];
    this->random_orth_matrices = new double[this->matrices_array_size];

    auto& p1 = this->create_task("compute_rotations");
    auto p1s_in_norms2 = this->template create_socket_in<T>(p1, "in_norms2", this->sub_vec_length);
    auto p1s_in_Y = this->template create_socket_in<T>(p1, "in_Y", this->n);
    auto p1s_in_U = this->template create_socket_in<float>(p1, "in_U", this->n);
    auto p1s_out_rotation_matrices =
      this->template create_socket_out<T>(p1, "out_rotation_matrices", this->matrices_array_size);
    this->create_codelet(p1,
                         [p1s_in_norms2, p1s_in_Y, p1s_in_U, p1s_out_rotation_matrices](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& rota_mat_cmp = static_cast<Rotation_matrices_computer&>(m);
                             rota_mat_cmp._compute_rotations(t[p1s_in_norms2].template get_dataptr<const T>(),
                                                             t[p1s_in_Y].template get_dataptr<const T>(),
                                                             t[p1s_in_U].template get_dataptr<const float>(),
                                                             t[p1s_out_rotation_matrices].template get_dataptr<T>(),
                                                             frame_id);
                             return runtime::status_t::SUCCESS;
                         });

    auto& p2 = this->create_task("compute_reverse_rotations");
    auto p2s_in_rotation_matrices =
      this->template create_socket_in<T>(p2, "in_rotation_matrices", this->matrices_array_size);
    auto p2s_in_X = this->template create_socket_in<T>(p2, "in_X", this->n);
    auto p2s_out_rotated_X = this->template create_socket_out<T>(p2, "out_rotated_X", this->n);
    this->create_codelet(
      p2,
      [p2s_in_rotation_matrices, p2s_in_X, p2s_out_rotated_X](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& rota_mat_cmp = static_cast<Rotation_matrices_computer&>(m);
          rota_mat_cmp._compute_reverse_rotations(t[p2s_in_rotation_matrices].template get_dataptr<const T>(),
                                                  t[p2s_in_X].template get_dataptr<const T>(),
                                                  t[p2s_out_rotated_X].template get_dataptr<T>(),
                                                  frame_id);
          return runtime::status_t::SUCCESS;
      });
}

template<typename T>
Rotation_matrices_computer<T>::~Rotation_matrices_computer()
{
    delete[] this->random_matrices;
    delete[] this->random_orth_matrices;
}

template<typename T>
Rotation_matrices_computer<T>*
Rotation_matrices_computer<T>::clone() const
{
    auto m = new Rotation_matrices_computer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
void
Rotation_matrices_computer<T>::deep_copy(const Rotation_matrices_computer<T>& m)
{
    spu::module::Stateful::deep_copy(m);

    this->random_matrices = new double[this->matrices_array_size];
    this->random_orth_matrices = new double[this->matrices_array_size];

    if (m.random_matrices != nullptr)
    {
        std::copy(m.random_matrices, m.random_matrices + this->matrices_array_size, this->random_matrices);
    }

    if (m.random_orth_matrices != nullptr)
    {
        std::copy(
          m.random_orth_matrices, m.random_orth_matrices + this->matrices_array_size, this->random_orth_matrices);
    }

    if (m.normal_gen != nullptr)
    {
        this->normal_gen.reset(m.normal_gen->clone());
    }
}

template<typename T>
size_t
Rotation_matrices_computer<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Rotation_matrices_computer<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
Rotation_matrices_computer<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
void
Rotation_matrices_computer<T>::set_seed(const int seed)
{
    this->normal_gen->set_seed(seed);
}

template<typename T>
void
Rotation_matrices_computer<T>::compute_rotations(const T* in_norms2,
                                                 const T* in_Y,
                                                 const float* in_U,
                                                 T* out_rotation_matrices,
                                                 const int frame_id,
                                                 const bool managed_memory)
{
    (*this)[rotation_mt_cmp::sck::compute_rotations::in_norms2].bind(in_norms2);
    (*this)[rotation_mt_cmp::sck::compute_rotations::in_Y].bind(in_Y);
    (*this)[rotation_mt_cmp::sck::compute_rotations::in_U].bind(in_U);
    (*this)[rotation_mt_cmp::sck::compute_rotations::out_rotation_matrices].bind(out_rotation_matrices);
    (*this)[rotation_mt_cmp::tsk::compute_rotations].exec(frame_id, managed_memory);
}

static inline void
generate_random_orthogonal_matrices(const size_t sub_vec_length,
                                    const size_t d,
                                    aff3ct::tools::Gaussian_noise_generator_std<double>& normal_gen,
                                    double* random_matrices,
                                    double* random_orth_matrices)
{
    // QR Eigen object allocation
    Eigen::HouseholderQR<Eigen::MatrixXd> _QR(d, d);

    for (size_t i = 0; i < sub_vec_length; i++)
    {
        // Mapping flatten pointers into Eigen objects (here matrices)
        Eigen::Map<Eigen::MatrixXd> _A(random_matrices + i * d * d, d, d);
        Eigen::Map<Eigen::MatrixXd> _Q_out(random_orth_matrices + i * d * d, d, d);

        // Generate random matrix with i.i.d. standard normal entries (basis for QR decomposition)
        normal_gen.generate(_A.data(), d * d, 1.0, 0.0);

        // QR decomposition
        _QR.compute(_A);
        // Orthogonal matrix (Q) extraction
        _QR.householderQ().evalTo(_Q_out);
    }
}

template<typename T>
void
Rotation_matrices_computer<T>::_compute_rotations(const T* in_norms2,
                                                  const T* in_Y,
                                                  const float* in_U,
                                                  T* out_rotation_matrices,
                                                  const size_t /*frame_id*/)
{
    // Generates random orthogonal
    generate_random_orthogonal_matrices(
      this->sub_vec_length, this->d, *this->normal_gen, this->random_matrices, this->random_orth_matrices);

    // Eigen identity matrix
    const Eigen::MatrixXd ID = Eigen::MatrixXd::Identity(this->d, this->d);

    // Mapping flatten pointers into Eigen objects
    Eigen::VectorXd Q_y(this->d);
    Eigen::VectorXd u_scaled(this->d);
    Eigen::VectorXd a(this->d);
    Eigen::MatrixXd _S(this->d, this->d);

    std::vector<double> U_double(this->n);

    // double type conversion
    for (size_t i = 0; i < this->n; i++)
    {
        U_double[i] = static_cast<double>(in_U[i]);
    }

    for (size_t i = 0; i < this->sub_vec_length; ++i)
    {
        // Mapping flatten pointers into Eigen objects
        Eigen::Map<const Eigen::VectorXd> y(in_Y + i * this->d, this->d);
        Eigen::Map<const Eigen::VectorXd> u(U_double.data() + i * this->d, this->d);

        // Mapping flatten pointers into Eigen objects
        Eigen::Map<Eigen::MatrixXd> random_orth_matrix(this->random_orth_matrices + i * d * d, d, d);

        const Eigen::MatrixXd& _Q = random_orth_matrix;

        // Householder transformations

        Q_y.noalias() = _Q * y;

        // Scale norm of u so it matches the norm of y
        const double scale = std::sqrt(in_norms2[i] / static_cast<double>(this->d));
        u_scaled.noalias() = u * scale;

        a.noalias() = Q_y - u_scaled;

        const double a_norm2 = a.squaredNorm();

        _S = ID;
        _S.noalias() -= (2.0 / a_norm2) * (a * a.transpose());

        // Compose random orthogonal transformation (_Q) and Householder reflection matrix (_S)
        Eigen::Map<Eigen::MatrixXd> _R(out_rotation_matrices + i * this->d * this->d, this->d, this->d);
        _R.noalias() = _S * _Q;
    }
}

template<typename T>
void
Rotation_matrices_computer<T>::compute_reverse_rotations(const T* in_rotation_matrices,
                                                         const T* in_X,
                                                         T* out_rotated_X,
                                                         const int frame_id,
                                                         const bool managed_memory)
{
    (*this)[rotation_mt_cmp::sck::compute_reverse_rotations::in_rotation_matrices].bind(in_rotation_matrices);
    (*this)[rotation_mt_cmp::sck::compute_reverse_rotations::in_X].bind(in_X);
    (*this)[rotation_mt_cmp::sck::compute_reverse_rotations::out_rotated_X].bind(out_rotated_X);
    (*this)[rotation_mt_cmp::tsk::compute_reverse_rotations].exec(frame_id, managed_memory);
}

template<typename T>
void
Rotation_matrices_computer<T>::_compute_reverse_rotations(const T* in_rotation_matrices,
                                                          const T* in_X,
                                                          T* out_rotated_X,
                                                          const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; ++i)
    {
        // Mapping flatten pointers into Eigen objects
        Eigen::Map<const Eigen::MatrixXd> _R(in_rotation_matrices + i * this->d * this->d, this->d, this->d);
        Eigen::Map<const Eigen::VectorXd> x(in_X + i * this->d, this->d);
        Eigen::Map<Eigen::VectorXd> y(out_rotated_X + i * this->d, this->d);

        // Compute reverse rotation
        y.noalias() = _R * x;
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Rotation_matrices_computer<double>;
// ==================================================================================== explicit template instantiation
