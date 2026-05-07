/*!
 * \file
 * \brief Class module::Rotation_matrices_computer.
 */
#ifndef ROTATION_MATRICES_COMPUTER_HPP_
#define ROTATION_MATRICES_COMPUTER_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace rotation_mt_cmp
{
enum class tsk : size_t
{
    compute_rotations,
    compute_reverse_rotations,
    SIZE
};

namespace sck
{
enum class compute_rotations : size_t
{
    in_norms2,
    in_Y,
    in_U,
    out_rotation_matrices,
    status
};

enum class compute_reverse_rotations : size_t
{
    in_rotation_matrices,
    in_X,
    out_rotated_X,
    status
};
}
}

template<typename T = double>
class Rotation_matrices_computer : public Stateful
{
  public:
    inline runtime::Task& operator[](const rotation_mt_cmp::tsk t);
    inline runtime::Socket& operator[](const rotation_mt_cmp::sck::compute_rotations s);
    inline runtime::Socket& operator[](const rotation_mt_cmp::sck::compute_reverse_rotations s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;
    int seed;

  private:
    const size_t matrices_array_size;
    double* random_matrices;
    double* random_orth_matrices;
    std::shared_ptr<aff3ct::tools::Gaussian_noise_generator_std<T>> normal_gen;

  public:
    Rotation_matrices_computer(const size_t n,
                               const size_t d,
                               const size_t sub_vec_length,
                               aff3ct::tools::Gaussian_noise_generator_std<T>& generator);
    virtual ~Rotation_matrices_computer();
    virtual Rotation_matrices_computer* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;

    virtual void set_seed(const int seed);

    template<class A1 = std::allocator<T>, class A2 = std::allocator<float>>
    void compute_rotations(const std::vector<T, A1>& in_norms2,
                           const std::vector<T, A1>& in_Y,
                           const std::vector<float, A2>& in_U,
                           std::vector<T, A1>& out_rotation_matrices,
                           const int frame_id = -1,
                           const bool managed_memory = true);

    void compute_rotations(const T* in_norms2,
                           const T* in_Y,
                           const float* in_U,
                           T* out_rotation_matrices,
                           const int frame_id = -1,
                           const bool managed_memory = true);

    template<class A = std::allocator<T>>
    void compute_reverse_rotations(const std::vector<T, A>& in_rotation_matrices,
                                   const std::vector<T, A>& in_X,
                                   std::vector<T, A>& out_rotated_X,
                                   const int frame_id = -1,
                                   const bool managed_memory = true);

    void compute_reverse_rotations(const T* in_rotation_matrices,
                                   const T* in_X,
                                   T* out_rotated_X,
                                   const int frame_id = -1,
                                   const bool managed_memory = true);

  protected:
    virtual void _compute_rotations(const T* in_norms2,
                                    const T* in_Y,
                                    const float* in_U,
                                    T* out_rotation_matrices,
                                    const size_t frame_id);

    virtual void _compute_reverse_rotations(const T* in_rotation_matrices,
                                            const T* in_X,
                                            T* out_rotated_X,
                                            const size_t frame_id);

    virtual void deep_copy(const Rotation_matrices_computer<T>& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Rotation_matrices_computer.hxx"
#endif

#endif
