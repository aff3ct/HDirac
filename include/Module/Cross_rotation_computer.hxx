#include "Module/Cross_rotation_computer.hpp"

namespace CVQKD
{
namespace module
{
template<typename T>
runtime::Task&
Cross_rotation_computer<T>::operator[](const cross_rot_cmp::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Cross_rotation_computer<T>::operator[](const cross_rot_cmp::sck::compute_cross_rot s)
{
    return Module::operator[]((size_t)cross_rot_cmp::tsk::compute_cross_rot)[(size_t)s];
}

template<typename T>
runtime::Socket&
Cross_rotation_computer<T>::operator[](const cross_rot_cmp::sck::compute_reverse_cross_rot s)
{
    return Module::operator[]((size_t)cross_rot_cmp::tsk::compute_reverse_cross_rot)[(size_t)s];
}

template<typename T>
runtime::Socket&
Cross_rotation_computer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Cross_rotation_computer<T>::compute_cross_rot(const std::vector<float, A>& in_U,
                                              const std::vector<T, A>& in_Y,
                                              std::vector<T, A>& out_Y_frob_norms,
                                              std::vector<T, A>& out_cross_rotations,
                                              const int frame_id,
                                              const bool managed_memory)
{
    (*this)[cross_rot_cmp::sck::compute_cross_rot::in_U].bind(in_U);
    (*this)[cross_rot_cmp::sck::compute_cross_rot::in_Y].bind(in_Y);
    (*this)[cross_rot_cmp::sck::compute_cross_rot::out_Y_frob_norms].bind(out_Y_frob_norms);
    (*this)[cross_rot_cmp::sck::compute_cross_rot::out_cross_rotations].bind(out_cross_rotations);
    (*this)[cross_rot_cmp::tsk::compute_cross_rot].exec(frame_id, managed_memory);
}

template<typename T>
template<class A>
void
Cross_rotation_computer<T>::compute_reverse_cross_rot(const std::vector<T, A>& in_cross_rotations,
                                                      const std::vector<T, A>& in_X,
                                                      std::vector<T, A>& out_reverse_cross_rotations,
                                                      const int frame_id,
                                                      const bool managed_memory)
{
    (*this)[cross_rot_cmp::sck::compute_reverse_cross_rot::in_cross_rotations].bind(in_cross_rotations);
    (*this)[cross_rot_cmp::sck::compute_reverse_cross_rot::in_X].bind(in_X);
    (*this)[cross_rot_cmp::sck::compute_reverse_cross_rot::out_reverse_cross_rotations].bind(
      out_reverse_cross_rotations);
    (*this)[cross_rot_cmp::tsk::compute_reverse_cross_rot].exec(frame_id, managed_memory);
}

}
}
