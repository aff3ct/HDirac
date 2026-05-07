/*!
 * \file
 * \brief Class module::Cross_rotation_computer.
 */
#ifndef CROSS_ROTATION_COMPUTER_HPP_
#define CROSS_ROTATION_COMPUTER_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace cross_rot_cmp
{
enum class tsk : size_t
{
    compute_cross_rot,
    compute_reverse_cross_rot,
    SIZE
};

namespace sck
{
enum class compute_cross_rot : size_t
{
    in_U,
    in_Y,
    out_Y_frob_norms,
    out_cross_rotations,
    status
};

enum class compute_reverse_cross_rot : size_t
{
    in_cross_rotations,
    in_X,
    out_reverse_cross_rotations,
    status
};
}
}

template<typename T = double>
class Cross_rotation_computer
  : public Stateful
  , public spu::tools::Interface_set_seed
{
  public:
    inline runtime::Task& operator[](const cross_rot_cmp::tsk t);
    inline runtime::Socket& operator[](const cross_rot_cmp::sck::compute_cross_rot s);
    inline runtime::Socket& operator[](const cross_rot_cmp::sck::compute_reverse_cross_rot s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;

  private:
    std::shared_ptr<aff3ct::tools::Gaussian_noise_generator_std<T>> normal_gen;
    double* u_tilde;      // size : n
    double* y_tilde;      // size : n
    double* temp_y;       // size : 8
    double* temp_y_tilde; // size : 8
    double* temp_u_tilde; // size : 8
    double* temp_u;       // size : 8

    double* x_tilde;      // size : n
    double* temp_x_tilde; // size : 8
    double* temp_x;       // size : 8

  public:
    Cross_rotation_computer(const size_t n,
                            const size_t d,
                            const size_t sub_vec_length,
                            aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen);
    virtual ~Cross_rotation_computer();
    virtual Cross_rotation_computer* clone() const;

    virtual void set_seed(const int seed);

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;

    template<class A = std::allocator<T>>
    void compute_cross_rot(const std::vector<float, A>& in_U,
                           const std::vector<T, A>& in_Y,
                           std::vector<T, A>& out_Y_frob_norms,
                           std::vector<T, A>& out_cross_rotations,
                           const int frame_id = -1,
                           const bool managed_memory = true);

    void compute_cross_rot(const float* in_U,
                           const T* in_Y,
                           T* out_Y_frob_norms,
                           T* out_cross_rotations,
                           const int frame_id = -1,
                           const bool managed_memory = true);

    template<class A = std::allocator<T>>
    void compute_reverse_cross_rot(const std::vector<T, A>& in_cross_rotations,
                                   const std::vector<T, A>& in_X,
                                   std::vector<T, A>& out_reverse_cross_rotations,
                                   const int frame_id = -1,
                                   const bool managed_memory = true);

    void compute_reverse_cross_rot(const T* in_cross_rotations,
                                   const T* in_X,
                                   T* out_reverse_cross_rotations,
                                   const int frame_id = -1,
                                   const bool managed_memory = true);

  protected:
    virtual void _compute_cross_rot(const float* in_U,
                                    const T* in_Y,
                                    T* out_Y_frob_norms,
                                    T* out_cross_rotations,
                                    const size_t frame_id);

    virtual void _compute_reverse_cross_rot(const T* in_cross_rotations,
                                            const T* in_X,
                                            T* out_reverse_cross_rotations,
                                            const size_t frame_id);

    virtual void deep_copy(const Cross_rotation_computer<T>& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Cross_rotation_computer.hxx"
#endif

#endif
