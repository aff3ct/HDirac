#include <sstream>

#include "Module/LLRs_computer.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
LLRs_computer<T>::LLRs_computer(const size_t n,
                                const size_t d,
                                const size_t sub_vec_length,
                                const float signal_variance)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
  , signal_variance(signal_variance)
{
    const std::string name = "LLRs_computer";
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

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (signal_variance < 1.0f)
    {
        std::stringstream message;
        message << "'signal_variance' has to be greater than or equal to 1.0 ('signal_variance' = " << signal_variance
                << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("compute_LLRs");
    auto p1s_in_snr = this->template create_socket_in<float>(p1, "in_snr", 1);
    auto p1s_in_rotated_X = this->template create_socket_in<T>(p1, "in_rotated_X", this->n);
    auto p1s_in_norms2 = this->template create_socket_in<T>(p1, "in_norms2", this->sub_vec_length);
    auto p1s_out_LLRs = this->template create_socket_out<float>(p1, "out_LLRs", this->n);
    this->create_codelet(p1,
                         [p1s_in_snr, p1s_in_rotated_X, p1s_in_norms2, p1s_out_LLRs](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& LLRs_cmp = static_cast<LLRs_computer&>(m);
                             LLRs_cmp._compute_LLRs(t[p1s_in_snr].template get_dataptr<const float>(),
                                                    t[p1s_in_rotated_X].template get_dataptr<const T>(),
                                                    t[p1s_in_norms2].template get_dataptr<const T>(),
                                                    t[p1s_out_LLRs].template get_dataptr<float>(),
                                                    frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
LLRs_computer<T>*
LLRs_computer<T>::clone() const
{
    auto m = new LLRs_computer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
LLRs_computer<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
LLRs_computer<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
LLRs_computer<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
float
LLRs_computer<T>::get_signal_variance() const
{
    return this->signal_variance;
}

template<typename T>
void
LLRs_computer<T>::compute_LLRs(const float* in_snr,
                               const T* in_rotated_X,
                               const T* in_norms2,
                               float* out_LLRs,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[LLR_cmp::sck::compute_LLRs::in_snr].bind(in_snr);
    (*this)[LLR_cmp::sck::compute_LLRs::in_rotated_X].bind(in_rotated_X);
    (*this)[LLR_cmp::sck::compute_LLRs::in_norms2].bind(in_norms2);
    (*this)[LLR_cmp::sck::compute_LLRs::out_LLRs].bind(out_LLRs);
    (*this)[LLR_cmp::tsk::compute_LLRs].exec(frame_id, managed_memory);
}

template<typename T>
void
LLRs_computer<T>::_compute_LLRs(const float* in_snr,
                                const T* in_rotated_X,
                                const T* in_norms2,
                                float* out_LLRs,
                                const size_t /*frame_id*/)
{
    const float CHANNEL_VAR = this->signal_variance / *in_snr;

    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        const T SCALE = 2.0 / (CHANNEL_VAR * std::sqrt(static_cast<T>(this->d) / in_norms2[i]));

        for (size_t j = 0; j < this->d; j++)
        {
            out_LLRs[i * this->d + j] = static_cast<float>(in_rotated_X[i * this->d + j] * SCALE);
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::LLRs_computer<double>;
// ==================================================================================== explicit template instantiation
