/*!
 * \file
 * \brief Class module::Norms2_computer.
 */
#ifndef NORMS_COMPUTER_HPP_
#define NORMS_COMPUTER_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace norms2_cmp
{
enum class tsk : size_t
{
    compute_norms2,
    SIZE
};

namespace sck
{
enum class compute_norms2 : size_t
{
    in_Y,
    out_norms2,
    status
};
}
}

template<typename T = double>
class Norms2_computer : public Stateful
{
  public:
    inline runtime::Task& operator[](const norms2_cmp::tsk t);
    inline runtime::Socket& operator[](const norms2_cmp::sck::compute_norms2 s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;

  public:
    Norms2_computer(const size_t n, const size_t d, const size_t sub_vec_length);
    virtual ~Norms2_computer() = default;
    virtual Norms2_computer* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;

    template<class A = std::allocator<T>>
    void compute_norms2(const std::vector<T, A>& in_Y,
                        std::vector<T, A>& out_norms2,
                        const int frame_id = -1,
                        const bool managed_memory = true);

    void compute_norms2(const T* in_Y, T* out_norms2, const int frame_id = -1, const bool managed_memory = true);

  protected:
    virtual void _compute_norms2(const T* in_Y, T* out_norms2, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Norms2_computer.hxx"
#endif

#endif