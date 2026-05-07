/*!
 * \file
 * \brief Class module::LLRs_computer_low_dimensions.
 */
#ifndef LLRS_COMPUTER_LOW_DIMENSIONS_HPP_
#define LLRS_COMPUTER_LOW_DIMENSIONS_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace LLR_cmp_low_d
{
enum class tsk : size_t
{
    compute_LLRs_low_d,
    SIZE
};

namespace sck
{
enum class compute_LLRs_low_d : size_t
{
    in_snr,
    in_X,
    in_quotients,
    out_LLRs,
    status
};
}
}

template<typename T = double>
class LLRs_computer_low_dimensions : public Stateful
{
  public:
    inline runtime::Task& operator[](const LLR_cmp_low_d::tsk t);
    inline runtime::Socket& operator[](const LLR_cmp_low_d::sck::compute_LLRs_low_d s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;
    const float signal_variance;

  public:
    LLRs_computer_low_dimensions(const size_t n,
                                 const size_t d,
                                 const size_t sub_vec_length,
                                 const float signal_variance = 1.0f);
    virtual ~LLRs_computer_low_dimensions() = default;
    virtual LLRs_computer_low_dimensions* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;
    float get_signal_variance() const;

    template<class A1 = std::allocator<float>, class A2 = std::allocator<T>>
    void compute_LLRs_low_d(const std::vector<float, A1>& in_snr,
                            const std::vector<T, A2>& in_X,
                            const std::vector<T, A2>& in_quotients,
                            std::vector<float, A1>& out_LLRs,
                            const int frame_id = -1,
                            const bool managed_memory = true);

    void compute_LLRs_low_d(const float* in_snr,
                            const T* in_X,
                            const T* in_quotients,
                            float* out_LLRs,
                            const int frame_id = -1,
                            const bool managed_memory = true);

  protected:
    virtual void _compute_LLRs_low_d(const float* in_snr,
                                     const T* in_X,
                                     const T* in_quotients,
                                     float* out_LLRs,
                                     const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/LLRs_computer_low_dimensions.hxx"
#endif

#endif