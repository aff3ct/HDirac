/*!
 * \file
 * \brief Class module::LLRs_computer_cross_rot.
 */
#ifndef LLRS_COMPUTER_CROSS_ROT_HPP_
#define LLRS_COMPUTER_CROSS_ROT_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace LLR_cmp_cr
{
enum class tsk : size_t
{
    compute_LLRs_cr,
    SIZE
};

namespace sck
{
enum class compute_LLRs_cr : size_t
{
    in_snr,
    in_rotated_X,
    in_Y_frob_norms,
    out_LLRs,
    status
};
}
}

template<typename T = double>
class LLRs_computer_cross_rot : public Stateful
{
  public:
    inline runtime::Task& operator[](const LLR_cmp_cr::tsk t);
    inline runtime::Socket& operator[](const LLR_cmp_cr::sck::compute_LLRs_cr s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;
    const float signal_variance;

  public:
    LLRs_computer_cross_rot(const size_t n,
                            const size_t d,
                            const size_t sub_vec_length,
                            const float signal_variance = 1.0f);
    virtual ~LLRs_computer_cross_rot() = default;
    virtual LLRs_computer_cross_rot* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;
    float get_signal_variance() const;

    template<class A1 = std::allocator<float>, class A2 = std::allocator<T>>
    void compute_LLRs_cr(const std::vector<float, A1>& in_snr,
                         const std::vector<T, A2>& in_rotated_X,
                         const std::vector<T, A2>& in_Y_frob_norms,
                         std::vector<float, A1>& out_LLRs,
                         const int frame_id = -1,
                         const bool managed_memory = true);

    void compute_LLRs_cr(const float* in_snr,
                         const T* in_rotated_X,
                         const T* in_Y_frob_norms,
                         float* out_LLRs,
                         const int frame_id = -1,
                         const bool managed_memory = true);

  protected:
    virtual void _compute_LLRs_cr(const float* in_snr,
                                  const T* in_rotated_X,
                                  const T* in_Y_frob_norms,
                                  float* out_LLRs,
                                  const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/LLRs_computer_cross_rot.hxx"
#endif

#endif