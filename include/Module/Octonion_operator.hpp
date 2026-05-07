/*!
 * \file
 * \brief Class module::Octonion_operator.
 */
#ifndef OCTONION_OPERATOR_HPP_
#define OCTONION_OPERATOR_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace octon_oper
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
    out_octonion_products,
    status
};

enum class divide : size_t
{
    in_octonion_products,
    in_X,
    out_octonion_quotients,
    status
};
}
}

template<typename T = double>
class Octonion_operator : public Stateful
{
  public:
    inline runtime::Task& operator[](const octon_oper::tsk t);
    inline runtime::Socket& operator[](const octon_oper::sck::multiply s);
    inline runtime::Socket& operator[](const octon_oper::sck::divide s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;

  public:
    Octonion_operator(const size_t n, const size_t d, const size_t sub_vec_length);
    virtual ~Octonion_operator() = default;
    virtual Octonion_operator* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;

    template<class A = std::allocator<T>>
    void multiply(const std::vector<float, A>& in_U,
                  const std::vector<T, A>& in_Y,
                  std::vector<T, A>& out_octonion_products,
                  const int frame_id = -1,
                  const bool managed_memory = true);

    void multiply(const float* in_U,
                  const T* in_Y,
                  T* out_octonion_products,
                  const int frame_id = -1,
                  const bool managed_memory = true);

    template<class A = std::allocator<T>>
    void divide(const std::vector<T, A>& in_octonion_products,
                const std::vector<T, A>& in_X,
                std::vector<T, A>& out_octonion_quotients,
                const int frame_id = -1,
                const bool managed_memory = true);

    void divide(const T* in_octonion_products,
                const T* in_X,
                T* out_octonion_quotients,
                const int frame_id = -1,
                const bool managed_memory = true);

  protected:
    virtual void _multiply(const float* in_U, const T* in_Y, T* out_octonion_products, const size_t frame_id);
    virtual void _divide(const T* in_octonion_products,
                         const T* in_X,
                         T* out_octonion_quotients,
                         const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Octonion_operator.hxx"
#endif

#endif
