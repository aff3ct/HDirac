#include "Module/Real_operator.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Real_operator<T>::operator[](const real_mapper::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Real_operator<T>::operator[](const real_mapper::sck::map s)
{
    return Module::operator[]((size_t)real_mapper::tsk::map)[(size_t)s];
}

template<typename T>
runtime::Socket&
Real_operator<T>::operator[](const real_mapper::sck::demap s)
{
    return Module::operator[]((size_t)real_mapper::tsk::demap)[(size_t)s];
}

template<typename T>
runtime::Socket&
Real_operator<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
Real_operator<T>::map(const std::vector<float, A1>& in_U,
                      const std::vector<T, A2>& in_Y,
                      std::vector<T, A2>& out_mappings,
                      const int frame_id,
                      const bool managed_memory)
{
    (*this)[real_mapper::sck::map::in_U].bind(in_U);
    (*this)[real_mapper::sck::map::in_Y].bind(in_Y);
    (*this)[real_mapper::sck::map::out_mappings].bind(out_mappings);
    (*this)[real_mapper::tsk::map].exec(frame_id, managed_memory);
}

template<typename T>
template<class A>
void
Real_operator<T>::demap(const std::vector<T, A>& in_mappings,
                        const std::vector<T, A>& in_X,
                        std::vector<T, A>& out_demappings,
                        const int frame_id,
                        const bool managed_memory)
{
    (*this)[real_mapper::sck::demap::in_mappings].bind(in_mappings);
    (*this)[real_mapper::sck::demap::in_X].bind(in_X);
    (*this)[real_mapper::sck::demap::out_demappings].bind(out_demappings);
    (*this)[real_mapper::tsk::demap].exec(frame_id, managed_memory);
}

}
}
