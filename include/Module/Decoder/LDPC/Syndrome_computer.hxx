#include "Module/Decoder/LDPC/Syndrome_computer.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Syndrome_computer<T>::operator[](const synd_cmp::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Syndrome_computer<T>::operator[](const synd_cmp::sck::compute_syndrome s)
{
    return Module::operator[]((size_t)synd_cmp::tsk::compute_syndrome)[(size_t)s];
}

template<typename T>
runtime::Socket&
Syndrome_computer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Syndrome_computer<T>::compute_syndrome(const std::vector<T, A>& in_codeword,
                                       std::vector<T, A>& out_syndrome,
                                       const int frame_id,
                                       const bool managed_memory)
{
    (*this)[synd_cmp::sck::compute_syndrome::in_codeword].bind(in_codeword);
    (*this)[synd_cmp::sck::compute_syndrome::out_syndrome].bind(out_syndrome);
    (*this)[synd_cmp::tsk::compute_syndrome].exec(frame_id, managed_memory);
}

}
}
