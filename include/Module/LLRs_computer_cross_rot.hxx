#include "Module/LLRs_computer_cross_rot.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
LLRs_computer_cross_rot<T>::operator[](const LLR_cmp_cr::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
LLRs_computer_cross_rot<T>::operator[](const LLR_cmp_cr::sck::compute_LLRs_cr s)
{
    return Module::operator[]((size_t)LLR_cmp_cr::tsk::compute_LLRs_cr)[(size_t)s];
}

template<typename T>
runtime::Socket&
LLRs_computer_cross_rot<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
LLRs_computer_cross_rot<T>::compute_LLRs_cr(const std::vector<float, A1>& in_snr,
                                            const std::vector<T, A2>& in_rotated_X,
                                            const std::vector<T, A2>& in_Y_frob_norms,
                                            std::vector<float, A1>& out_LLRs,
                                            const int frame_id,
                                            const bool managed_memory)
{
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::in_snr].bind(in_snr);
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::in_rotated_X].bind(in_rotated_X);
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::in_Y_frob_norms].bind(in_Y_frob_norms);
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::out_LLRs].bind(out_LLRs);
    (*this)[LLR_cmp_cr::tsk::compute_LLRs_cr].exec(frame_id, managed_memory);
}

}
}
