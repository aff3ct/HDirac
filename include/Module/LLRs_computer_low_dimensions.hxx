#include "Module/LLRs_computer_low_dimensions.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
LLRs_computer_low_dimensions<T>::operator[](const LLR_cmp_low_d::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
LLRs_computer_low_dimensions<T>::operator[](const LLR_cmp_low_d::sck::compute_LLRs_low_d s)
{
    return Module::operator[]((size_t)LLR_cmp_low_d::tsk::compute_LLRs_low_d)[(size_t)s];
}

template<typename T>
runtime::Socket&
LLRs_computer_low_dimensions<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
LLRs_computer_low_dimensions<T>::compute_LLRs_low_d(const std::vector<float, A1>& in_snr,
                                                    const std::vector<T, A2>& in_X,
                                                    const std::vector<T, A2>& in_quotients,
                                                    std::vector<float, A1>& out_LLRs,
                                                    const int frame_id,
                                                    const bool managed_memory)
{
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::in_snr].bind(in_snr);
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::in_X].bind(in_X);
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::in_quotients].bind(in_quotients);
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::out_LLRs].bind(out_LLRs);
    (*this)[LLR_cmp_low_d::tsk::compute_LLRs_low_d].exec(frame_id, managed_memory);
}

}
}
