/*!
 * \file
 * \brief Class module::Real_operator.
 */
#ifndef REAL_OPERATOR_HPP_
#define REAL_OPERATOR_HPP_

#include <aff3ct.hpp>

#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace module
{
namespace real_mapper
{
enum class tsk : size_t
{
    map,
    demap,
    SIZE
};

namespace sck
{
enum class map : size_t
{
    in_U,
    in_Y,
    out_mappings,
    status
};

enum class demap : size_t
{
    in_mappings,
    in_X,
    out_demappings,
    status
};
}
}

template<typename T = double>
class Real_operator : public Stateful
{
  public:
    inline runtime::Task& operator[](const real_mapper::tsk t);
    inline runtime::Socket& operator[](const real_mapper::sck::map s);
    inline runtime::Socket& operator[](const real_mapper::sck::demap s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n;
    const size_t d;
    const size_t sub_vec_length;

  public:
    Real_operator(const size_t n, const size_t d, const size_t sub_vec_length);
    virtual ~Real_operator() = default;
    virtual Real_operator* clone() const;

    size_t get_n() const;
    size_t get_d() const;
    size_t get_sub_vec_length() const;

    template<class A1 = std::allocator<float>, class A2 = std::allocator<T>>
    void map(const std::vector<float, A1>& in_U,
             const std::vector<T, A2>& in_Y,
             std::vector<T, A2>& out_mappings,
             const int frame_id = -1,
             const bool managed_memory = true);

    void map(const float* in_U,
             const T* in_Y,
             T* out_mappings,
             const int frame_id = -1,
             const bool managed_memory = true);

    template<class A = std::allocator<T>>
    void demap(const std::vector<T, A>& in_mappings,
               const std::vector<T, A>& in_X,
               std::vector<T, A>& out_demappings,
               const int frame_id = -1,
               const bool managed_memory = true);

    void demap(const T* in_mappings,
               const T* in_X,
               T* out_demappings,
               const int frame_id = -1,
               const bool managed_memory = true);

  protected:
    virtual void _map(const float* in_U, const T* in_Y, T* out_mappings, const size_t frame_id);
    virtual void _demap(const T* in_mappings, const T* in_X, T* out_demappings, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Real_operator.hxx"
#endif

#endif
