#include "Module/Octonion_operator.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Octonion_operator<T>::operator[](const octon_oper::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Octonion_operator<T>::operator[](const octon_oper::sck::multiply s)
{
    return Module::operator[]((size_t)octon_oper::tsk::multiply)[(size_t)s];
}

template<typename T>
runtime::Socket&
Octonion_operator<T>::operator[](const octon_oper::sck::divide s)
{
    return Module::operator[]((size_t)octon_oper::tsk::divide)[(size_t)s];
}

template<typename T>
runtime::Socket&
Octonion_operator<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Octonion_operator<T>::multiply(const std::vector<float, A>& in_U,
                               const std::vector<T, A>& in_Y,
                               std::vector<T, A>& out_octonion_products,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[octon_oper::sck::multiply::in_U].bind(in_U);
    (*this)[octon_oper::sck::multiply::in_Y].bind(in_Y);
    (*this)[octon_oper::sck::multiply::out_octonion_products].bind(out_octonion_products);
    (*this)[octon_oper::tsk::multiply].exec(frame_id, managed_memory);
}

template<typename T>
template<class A>
void
Octonion_operator<T>::divide(const std::vector<T, A>& in_octonion_products,
                             const std::vector<T, A>& in_X,
                             std::vector<T, A>& out_octonion_quotients,
                             const int frame_id,
                             const bool managed_memory)
{
    (*this)[octon_oper::sck::divide::in_octonion_products].bind(in_octonion_products);
    (*this)[octon_oper::sck::divide::in_X].bind(in_X);
    (*this)[octon_oper::sck::divide::out_octonion_quotients].bind(out_octonion_quotients);
    (*this)[octon_oper::tsk::divide].exec(frame_id, managed_memory);
}

}
}
