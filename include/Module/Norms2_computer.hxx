#include "Module/Norms2_computer.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Norms2_computer<T>::operator[](const norms2_cmp::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Norms2_computer<T>::operator[](const norms2_cmp::sck::compute_norms2 s)
{
    return Module::operator[]((size_t)norms2_cmp::tsk::compute_norms2)[(size_t)s];
}

template<typename T>
runtime::Socket&
Norms2_computer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Norms2_computer<T>::compute_norms2(const std::vector<T, A>& in_Y,
                                   std::vector<T, A>& out_norms2,
                                   const int frame_id,
                                   const bool managed_memory)
{
    (*this)[norms2_cmp::sck::compute_norms2::in_Y].bind(in_Y);
    (*this)[norms2_cmp::sck::compute_norms2::out_norms2].bind(out_norms2);
    (*this)[norms2_cmp::tsk::compute_norms2].exec(frame_id, managed_memory);
}

}
}
