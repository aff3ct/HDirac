#include "Module/LLRs_computer.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
LLRs_computer<T>::operator[](const LLR_cmp::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
LLRs_computer<T>::operator[](const LLR_cmp::sck::compute_LLRs s)
{
    return Module::operator[]((size_t)LLR_cmp::tsk::compute_LLRs)[(size_t)s];
}

template<typename T>
runtime::Socket&
LLRs_computer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
LLRs_computer<T>::compute_LLRs(const std::vector<float, A1>& in_snr,
                               const std::vector<T, A2>& in_rotated_X,
                               const std::vector<T, A2>& in_norms2,
                               std::vector<float, A1>& out_LLRs,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[LLR_cmp::sck::compute_LLRs::in_snr].bind(in_snr);
    (*this)[LLR_cmp::sck::compute_LLRs::in_rotated_X].bind(in_rotated_X);
    (*this)[LLR_cmp::sck::compute_LLRs::in_norms2].bind(in_norms2);
    (*this)[LLR_cmp::sck::compute_LLRs::out_LLRs].bind(out_LLRs);
    (*this)[LLR_cmp::tsk::compute_LLRs].exec(frame_id, managed_memory);
}

}
}
