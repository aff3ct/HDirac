#include "Module/LLRs_formatter.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
LLRs_formatter<T>::operator[](const LLRs_fmt::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
LLRs_formatter<T>::operator[](const LLRs_fmt::sck::format_LLRs s)
{
    return Module::operator[]((size_t)LLRs_fmt::tsk::format_LLRs)[(size_t)s];
}

template<typename T>
runtime::Socket&
LLRs_formatter<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
LLRs_formatter<T>::format_LLRs(const std::vector<int, A1>& in_U_ref,
                               const std::vector<T, A2>& in_LLRs,
                               std::vector<T, A2>& out_fmt_LLRs,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[LLRs_fmt::sck::format_LLRs::in_U_ref].bind(in_U_ref);
    (*this)[LLRs_fmt::sck::format_LLRs::in_LLRs].bind(in_LLRs);
    (*this)[LLRs_fmt::sck::format_LLRs::out_fmt_LLRs].bind(out_fmt_LLRs);
    (*this)[LLRs_fmt::tsk::format_LLRs].exec(frame_id, managed_memory);
}

}
}
