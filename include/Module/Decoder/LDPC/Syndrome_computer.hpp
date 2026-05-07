/*!
 * \file
 * \brief Class module::Syndrome_computer.
 */
#ifndef SYNDROME_COMPUTER_HPP_
#define SYNDROME_COMPUTER_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace synd_cmp
{
enum class tsk : size_t
{
    compute_syndrome,
    SIZE
};

namespace sck
{
enum class compute_syndrome : size_t
{
    in_codeword,
    out_syndrome,
    status
};
}
}

template<typename T = int>
class Syndrome_computer : public Stateful
{
  public:
    inline runtime::Task& operator[](const synd_cmp::tsk t);
    inline runtime::Socket& operator[](const synd_cmp::sck::compute_syndrome s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    size_t n;
    size_t r;

  private:
    const aff3ct::tools::Sparse_matrix& _H;

  public:
    Syndrome_computer(const size_t n, const size_t r, const aff3ct::tools::Sparse_matrix& _H);
    virtual ~Syndrome_computer() = default;
    virtual Syndrome_computer* clone() const;

    size_t get_n() const;
    size_t get_r() const;

    template<class A = std::allocator<T>>
    void compute_syndrome(const std::vector<T, A>& in_codeword,
                          std::vector<T, A>& out_syndrome,
                          const int frame_id = -1,
                          const bool managed_memory = true);

    void compute_syndrome(const T* in_codeword,
                          const T* out_syndrome,
                          const int frame_id = -1,
                          const bool managed_memory = true);

  protected:
    virtual void _compute_syndrome(const T* in_codeword, T* out_syndrome, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Decoder/LDPC/Syndrome_computer.hxx"
#endif

#endif
