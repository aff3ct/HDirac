#include "Module/Complex_operator.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Complex_operator<T>::operator[](const complx_oper::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Complex_operator<T>::operator[](const complx_oper::sck::multiply s)
{
    return Module::operator[]((size_t)complx_oper::tsk::multiply)[(size_t)s];
}

template<typename T>
runtime::Socket&
Complex_operator<T>::operator[](const complx_oper::sck::divide s)
{
    return Module::operator[]((size_t)complx_oper::tsk::divide)[(size_t)s];
}

template<typename T>
runtime::Socket&
Complex_operator<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
Complex_operator<T>::multiply(const std::vector<float, A1>& in_U,
                              const std::vector<T, A2>& in_Y,
                              std::vector<T, A2>& out_complex_products,
                              const int frame_id,
                              const bool managed_memory)
{
    (*this)[complx_oper::sck::multiply::in_U].bind(in_U);
    (*this)[complx_oper::sck::multiply::in_Y].bind(in_Y);
    (*this)[complx_oper::sck::multiply::out_complex_products].bind(out_complex_products);
    (*this)[complx_oper::tsk::multiply].exec(frame_id, managed_memory);
}

template<typename T>
template<class A>
void
Complex_operator<T>::divide(const std::vector<T, A>& in_complex_products,
                            const std::vector<T, A>& in_X,
                            std::vector<T, A>& out_complex_quotients,
                            const int frame_id,
                            const bool managed_memory)
{
    (*this)[complx_oper::sck::divide::in_complex_products].bind(in_complex_products);
    (*this)[complx_oper::sck::divide::in_X].bind(in_X);
    (*this)[complx_oper::sck::divide::out_complex_quotients].bind(out_complex_quotients);
    (*this)[complx_oper::tsk::divide].exec(frame_id, managed_memory);
}

}
}
