#include "Module/Quaternion_operator.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Quaternion_operator<T>::operator[](const quater_oper::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Quaternion_operator<T>::operator[](const quater_oper::sck::multiply s)
{
    return Module::operator[]((size_t)quater_oper::tsk::multiply)[(size_t)s];
}

template<typename T>
runtime::Socket&
Quaternion_operator<T>::operator[](const quater_oper::sck::divide s)
{
    return Module::operator[]((size_t)quater_oper::tsk::divide)[(size_t)s];
}

template<typename T>
runtime::Socket&
Quaternion_operator<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
Quaternion_operator<T>::multiply(const std::vector<float, A1>& in_U,
                                 const std::vector<T, A2>& in_Y,
                                 std::vector<T, A2>& out_quaternion_products,
                                 const int frame_id,
                                 const bool managed_memory)
{
    (*this)[quater_oper::sck::multiply::in_U].bind(in_U);
    (*this)[quater_oper::sck::multiply::in_Y].bind(in_Y);
    (*this)[quater_oper::sck::multiply::out_quaternion_products].bind(out_quaternion_products);
    (*this)[quater_oper::tsk::multiply].exec(frame_id, managed_memory);
}

template<typename T>
template<class A>
void
Quaternion_operator<T>::divide(const std::vector<T, A>& in_quaternion_products,
                               const std::vector<T, A>& in_X,
                               std::vector<T, A>& out_quaternion_quotients,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[quater_oper::sck::divide::in_quaternion_products].bind(in_quaternion_products);
    (*this)[quater_oper::sck::divide::in_X].bind(in_X);
    (*this)[quater_oper::sck::divide::out_quaternion_quotients].bind(out_quaternion_quotients);
    (*this)[quater_oper::tsk::divide].exec(frame_id, managed_memory);
}

}
}
