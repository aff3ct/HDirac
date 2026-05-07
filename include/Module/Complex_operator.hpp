/*!
 * \file
 * \brief Class module::Complex_operator.
 */
#ifndef COMPLEX_OPERATOR_HPP_
#define COMPLEX_OPERATOR_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace complx_oper
{
enum class tsk : size_t
{
    multiply,
    divide,
    SIZE
};

namespace sck
{
enum class multiply : size_t
{
    in_U,
    in_Y,
    out_complex_products,
    status
};

enum class divide : size_t
{
    in_complex_products,
    in_X,
    out_complex_quotients,
    status
};
}
}

template<typename T = double>
class Complex_operator : public Stateful
{
  public:
    inline runtime::Task& operator[](const complx_oper::tsk t);
    inline runtime::Socket& operator[](const complx_oper::sck::multiply s);
    inline runtime::Socket& operator[](const complx_oper::sck::divide s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;

  public:
    Complex_operator(const size_t n, const size_t d, const size_t sub_vec_length);
    virtual ~Complex_operator() = default;
    virtual Complex_operator* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;

    template<class A1 = std::allocator<float>, class A2 = std::allocator<T>>
    void multiply(const std::vector<float, A1>& in_U,
                  const std::vector<T, A2>& in_Y,
                  std::vector<T, A2>& out_complex_products,
                  const int frame_id = -1,
                  const bool managed_memory = true);

    void multiply(const float* in_U,
                  const T* in_Y,
                  T* out_complex_products,
                  const int frame_id = -1,
                  const bool managed_memory = true);

    template<class A = std::allocator<T>>
    void divide(const std::vector<T, A>& in_complex_products,
                const std::vector<T, A>& in_X,
                std::vector<T, A>& out_complex_quotients,
                const int frame_id = -1,
                const bool managed_memory = true);

    void divide(const T* in_complex_products,
                const T* in_X,
                T* out_complex_quotients,
                const int frame_id = -1,
                const bool managed_memory = true);

  protected:
    virtual void _multiply(const float* in_U, const T* in_Y, T* out_complex_products, const size_t frame_id);
    virtual void _divide(const T* in_complex_products, const T* in_X, T* out_complex_quotients, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Complex_operator.hxx"
#endif

#endif
