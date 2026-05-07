#include <sstream>

#include "Module/Octonion_operator.hpp"
#include "Tools/Helper.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Octonion_operator<T>::Octonion_operator(const size_t n, const size_t d, const size_t sub_vec_length)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
{
    const std::string name = "Octonion_operator";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d != 8)
    {
        std::stringstream message;
        message << "'d' should be equal to 2 ('d' = " << d << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("multiply");
    auto p1s_in_U = this->template create_socket_in<float>(p1, "in_U", this->n);
    auto p1s_in_Y = this->template create_socket_in<T>(p1, "in_Y", this->n);
    auto p1s_out_octonion_products = this->template create_socket_out<T>(p1, "out_octonion_products", this->n);
    this->create_codelet(
      p1,
      [p1s_in_U, p1s_in_Y, p1s_out_octonion_products](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& oct_mul = static_cast<Octonion_operator&>(m);
          oct_mul._multiply(t[p1s_in_U].template get_dataptr<const float>(),
                            t[p1s_in_Y].template get_dataptr<const T>(),
                            t[p1s_out_octonion_products].template get_dataptr<T>(),
                            frame_id);
          return runtime::status_t::SUCCESS;
      });

    auto& p2 = this->create_task("divide");
    auto p2s_in_octonion_products = this->template create_socket_in<T>(p2, "in_octonion_products", this->n);
    auto p2s_in_X = this->template create_socket_in<T>(p2, "in_X", this->n);
    auto p2s_out_octonion_quotients = this->template create_socket_out<T>(p2, "out_octonion_quotients", this->n);
    this->create_codelet(p2,
                         [p2s_in_octonion_products, p2s_in_X, p2s_out_octonion_quotients](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& oct_div = static_cast<Octonion_operator&>(m);
                             oct_div._divide(t[p2s_in_octonion_products].template get_dataptr<const T>(),
                                             t[p2s_in_X].template get_dataptr<const T>(),
                                             t[p2s_out_octonion_quotients].template get_dataptr<T>(),
                                             frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Octonion_operator<T>*
Octonion_operator<T>::clone() const
{
    auto m = new Octonion_operator(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
Octonion_operator<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Octonion_operator<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
Octonion_operator<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
void
Octonion_operator<T>::multiply(const float* in_U,
                               const T* in_Y,
                               T* out_octonion_products,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[octon_oper::sck::multiply::in_U].bind(in_U);
    (*this)[octon_oper::sck::multiply::in_Y].bind(in_Y);
    (*this)[octon_oper::sck::multiply::out_octonion_products].bind(out_octonion_products);
    (*this)[octon_oper::tsk::multiply].exec(frame_id, managed_memory);
}

template<typename T>
void
Octonion_operator<T>::divide(const T* in_octonion_products,
                             const T* in_X,
                             T* out_octonion_quotients,
                             const int frame_id,
                             const bool managed_memory)
{
    (*this)[octon_oper::sck::divide::in_octonion_products].bind(in_octonion_products);
    (*this)[octon_oper::sck::divide::in_X].bind(in_X);
    (*this)[octon_oper::sck::divide::out_octonion_quotients].bind(out_octonion_quotients);
    (*this)[octon_oper::tsk::divide].exec(frame_id, managed_memory);
}

template<typename T>
void
Octonion_operator<T>::_multiply(const float* in_U, const T* in_Y, T* out_octonion_products, const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        CVQKD::tools::octonion_multiplication(
          &in_U[i * this->d], &in_Y[i * this->d], &out_octonion_products[i * this->d]);
    }
}

template<typename T>
void
Octonion_operator<T>::_divide(const T* in_octonion_products,
                              const T* in_X,
                              T* out_octonion_quotients,
                              const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        CVQKD::tools::octonion_division(
          &in_octonion_products[i * this->d], &in_X[i * this->d], &out_octonion_quotients[i * this->d]);
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Octonion_operator<double>;
// ==================================================================================== explicit template instantiation
