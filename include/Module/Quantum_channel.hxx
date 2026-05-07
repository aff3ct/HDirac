#include "Module/Quantum_channel.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
Quantum_channel<T>::operator[](const q_channel::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Quantum_channel<T>::operator[](const q_channel::sck::add_noise s)
{
    return Module::operator[]((size_t)q_channel::tsk::add_noise)[(size_t)s];
}

template<typename T>
runtime::Socket&
Quantum_channel<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
Quantum_channel<T>::add_noise(const std::vector<float, A1>& in_snr,
                              const std::vector<T, A2>& in_X,
                              std::vector<T, A2>& out_Y,
                              const int frame_id,
                              const bool managed_memory)
{
    (*this)[q_channel::sck::add_noise::in_snr].bind(in_snr);
    (*this)[q_channel::sck::add_noise::in_X].bind(in_X);
    (*this)[q_channel::sck::add_noise::out_Y].bind(out_Y);
    (*this)[q_channel::tsk::add_noise].exec(frame_id, managed_memory);
}

}
}
