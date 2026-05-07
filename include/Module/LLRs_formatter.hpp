/*!
 * \file
 * \brief Class module::LLRs_formatter.
 */
#ifndef LLR_FORMATTER_HPP_
#define LLR_FORMATTER_HPP_

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace LLRs_fmt
{
enum class tsk : size_t
{
    format_LLRs,
    SIZE
};

namespace sck
{
enum class format_LLRs : size_t
{
    in_U_ref,
    in_LLRs,
    out_fmt_LLRs,
    status
};
}
}

template<typename T = float>
class LLRs_formatter : public Stateful
{
  public:
    inline runtime::Task& operator[](const LLRs_fmt::tsk t);
    inline runtime::Socket& operator[](const LLRs_fmt::sck::format_LLRs s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t r;
    const size_t fmt_n;

  private:
    const aff3ct::tools::Sparse_matrix& _P;
    int* syndrome;

  public:
    LLRs_formatter(const size_t n, const size_t r, const aff3ct::tools::Sparse_matrix& _P);
    virtual ~LLRs_formatter();
    virtual LLRs_formatter* clone() const;

    size_t get_n() const;
    size_t get_r() const;
    size_t get_fmt_n() const;

    template<class A1 = std::allocator<int>, class A2 = std::allocator<T>>
    void format_LLRs(const std::vector<int, A1>& in_U_ref,
                     const std::vector<T, A2>& in_LLRs,
                     std::vector<T, A2>& out_fmt_LLRs,
                     const int frame_id = -1,
                     const bool managed_memory = true);

    void format_LLRs(const int* in_U_ref,
                     const T* in_LLRs,
                     T* out_fmt_LLRs,
                     const int frame_id = -1,
                     const bool managed_memory = true);

  protected:
    void compute_syndrome(const int* ref_bits);

    virtual void _format_LLRs(const int* in_U_ref, const T* in_LLRs, T* out_fmt_LLRs, const size_t frame_id);

    virtual void deep_copy(const LLRs_formatter<T>& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/LLRs_formatter.hxx"
#endif

#endif