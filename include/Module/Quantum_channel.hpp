/*!
 * \file
 * \brief Class module::Quantum_channel.
 */
#ifndef QUANTUM_CHANNEL_HPP_
#define QUANTUM_CHANNEL_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace q_channel
{
enum class tsk : size_t
{
    add_noise,
    SIZE
};

namespace sck
{
enum class add_noise : size_t
{
    in_snr,
    in_X,
    out_Y,
    status
};
}
}

template<typename T = double>
class Quantum_channel
  : public Stateful
  , public spu::tools::Interface_set_seed
{
  public:
    inline runtime::Task& operator[](const q_channel::tsk t);
    inline runtime::Socket& operator[](const q_channel::sck::add_noise s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    int seed;
    const float signal_variance;

  private:
    std::shared_ptr<aff3ct::tools::Gaussian_noise_generator_std<T>> normal_gen;

  public:
    Quantum_channel(const size_t n,
                    aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen,
                    const float signal_variance = 1.0f);
    virtual ~Quantum_channel() = default;
    virtual Quantum_channel* clone() const;

    virtual void set_seed(const int seed);

    size_t get_n() const;
    float get_signal_variance() const;

    template<class A1 = std::allocator<float>, class A2 = std::allocator<T>>
    void add_noise(const std::vector<float, A1>& in_snr,
                   const std::vector<T, A2>& in_X,
                   std::vector<T, A2>& out_Y,
                   const int frame_id = -1,
                   const bool managed_memory = true);

    void add_noise(const float* in_snr,
                   const T* in_X,
                   T* out_Y,
                   const int frame_id = -1,
                   const bool managed_memory = true);

  protected:
    virtual void _add_noise(const float* in_snr, const T* in_X, T* out_Y, const size_t frame_id);
    virtual void deep_copy(const Quantum_channel<T>& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Quantum_channel.hxx"
#endif

#endif