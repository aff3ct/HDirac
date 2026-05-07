#include <sstream>

#include "Module/Norms2_computer.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Norms2_computer<T>::Norms2_computer(const size_t n, const size_t d, const size_t sub_vec_length)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
{
    const std::string name = "Norms2_computer";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d == 0)
    {
        std::stringstream message;
        message << "'d' has to be greater than 0 ('d' = " << d << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length == 0)
    {
        std::stringstream message;
        message << "'sub_vec_length' has to be greater than 0 ('sub_vec_length' = " << sub_vec_length << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("compute_norms2");
    auto p1s_in_Y = this->template create_socket_in<T>(p1, "in_Y", this->n);
    auto p1s_out_norms2 = this->template create_socket_out<T>(p1, "out_norms2", this->sub_vec_length);
    this->create_codelet(p1,
                         [p1s_in_Y, p1s_out_norms2](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& norms2_cmp = static_cast<Norms2_computer&>(m);
                             norms2_cmp._compute_norms2(t[p1s_in_Y].template get_dataptr<const T>(),
                                                        t[p1s_out_norms2].template get_dataptr<T>(),
                                                        frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Norms2_computer<T>*
Norms2_computer<T>::clone() const
{
    auto m = new Norms2_computer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
Norms2_computer<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Norms2_computer<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
Norms2_computer<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
void
Norms2_computer<T>::compute_norms2(const T* in_Y, T* out_norms2, const int frame_id, const bool managed_memory)
{
    (*this)[norms2_cmp::sck::compute_norms2::in_Y].bind(in_Y);
    (*this)[norms2_cmp::sck::compute_norms2::out_norms2].bind(out_norms2);
    (*this)[norms2_cmp::tsk::compute_norms2].exec(frame_id, managed_memory);
}

template<typename T>
void
Norms2_computer<T>::_compute_norms2(const T* in_Y, T* out_norms2, const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        out_norms2[i] = static_cast<T>(0.0);
        for (size_t j = 0; j < d; j++)
        {
            out_norms2[i] += pow(in_Y[i * d + j], 2);
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Norms2_computer<float>;
template class CVQKD::module::Norms2_computer<double>;
// ==================================================================================== explicit template instantiation
