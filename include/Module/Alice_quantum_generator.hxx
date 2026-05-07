#include "Module/Alice_quantum_generator.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
Alice_quantum_generator<T>::operator[](const gen::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Alice_quantum_generator<T>::operator[](const gen::sck::generate s)
{
    return Module::operator[]((size_t)gen::tsk::generate)[(size_t)s];
}

template<typename T>
runtime::Socket&
Alice_quantum_generator<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Alice_quantum_generator<T>::generate(std::vector<T, A>& out_X, const int frame_id, const bool managed_memory)
{
    (*this)[gen::sck::generate::out_X].bind(out_X);
    (*this)[gen::tsk::generate].exec(frame_id, managed_memory);
}

}
}
