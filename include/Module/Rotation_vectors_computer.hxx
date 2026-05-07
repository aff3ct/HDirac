#include "Module/Rotation_vectors_computer.hpp"

namespace CVQKD
{
namespace module
{

template<typename T>
runtime::Task&
Rotation_vectors_computer<T>::operator[](const rotation_vct_cmp::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Rotation_vectors_computer<T>::operator[](const rotation_vct_cmp::sck::compute_rotations s)
{
    return Module::operator[]((size_t)rotation_vct_cmp::tsk::compute_rotations)[(size_t)s];
}

template<typename T>
runtime::Socket&
Rotation_vectors_computer<T>::operator[](const rotation_vct_cmp::sck::compute_reverse_rotations s)
{
    return Module::operator[]((size_t)rotation_vct_cmp::tsk::compute_reverse_rotations)[(size_t)s];
}

template<typename T>
runtime::Socket&
Rotation_vectors_computer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A1, class A2>
void
Rotation_vectors_computer<T>::compute_rotations(const std::vector<T, A1>& in_norms2,
                                                const std::vector<T, A1>& in_Y,
                                                const std::vector<float, A2>& in_U,
                                                std::vector<T, A1>& out_rotation_vectors,
                                                const int frame_id,
                                                const bool managed_memory)
{
    (*this)[rotation_vct_cmp::sck::compute_rotations::in_norms2].bind(in_norms2);
    (*this)[rotation_vct_cmp::sck::compute_rotations::in_Y].bind(in_Y);
    (*this)[rotation_vct_cmp::sck::compute_rotations::in_U].bind(in_U);
    (*this)[rotation_vct_cmp::sck::compute_rotations::out_rotation_vectors].bind(out_rotation_vectors);
    (*this)[rotation_vct_cmp::tsk::compute_rotations].exec(frame_id, managed_memory);
}

template<typename T>
template<class A>
void
Rotation_vectors_computer<T>::compute_reverse_rotations(const std::vector<T, A>& in_rotation_vectors,
                                                        const std::vector<T, A>& in_X,
                                                        std::vector<T, A>& out_rotated_X,
                                                        const int frame_id,
                                                        const bool managed_memory)
{
    (*this)[rotation_vct_cmp::sck::compute_reverse_rotations::in_rotation_vectors].bind(in_rotation_vectors);
    (*this)[rotation_vct_cmp::sck::compute_reverse_rotations::in_X].bind(in_X);
    (*this)[rotation_vct_cmp::sck::compute_reverse_rotations::out_rotated_X].bind(out_rotated_X);
    (*this)[rotation_vct_cmp::tsk::compute_reverse_rotations].exec(frame_id, managed_memory);
}

}
}
