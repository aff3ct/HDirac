/*!
 * \file
 * \brief Class module::Alice_quantum_generator.
 */
#ifndef ALICE_QUANTUM_GENERATOR_HPP_
#define ALICE_QUANTUM_GENERATOR_HPP_

#include <cstdint>
#include <vector>

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace gen
{
enum class tsk : size_t
{
    generate,
    SIZE
};

namespace sck
{
enum class generate : size_t
{
    out_X
};
}
}

template<typename T = double>
class Alice_quantum_generator
  : public Stateful
  , public spu::tools::Interface_set_seed
{
  public:
    inline runtime::Task& operator[](const gen::tsk t);
    inline runtime::Socket& operator[](const gen::sck::generate s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const float alice_var;
    const size_t n;

  private:
    std::shared_ptr<aff3ct::tools::Gaussian_noise_generator_std<T>> normal_gen;

  public:
    Alice_quantum_generator(const size_t n,
                            const float alice_var,
                            aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen);
    virtual ~Alice_quantum_generator() = default;
    virtual Alice_quantum_generator* clone() const;

    virtual void set_seed(const int seed);

    float get_alice_var() const;
    size_t get_n() const;

    template<class A = std::allocator<T>>
    void generate(std::vector<T, A>& out_X, const int frame_id = -1, const bool managed_memory = true);

    void generate(T* out_X, const int frame_id = -1, const bool managed_memory = true);

  protected:
    virtual void _generate(T* out_X, const size_t frame_id);
    virtual void deep_copy(const Alice_quantum_generator<T>& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Alice_quantum_generator.hxx"
#endif

#endif /* ALICE_QUANTUM_GENERATOR_HPP_ */
