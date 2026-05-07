#include <sstream>

#include "Module/LLRs_computer_cross_rot.hpp"
#include "Tools/Helper.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
LLRs_computer_cross_rot<T>::LLRs_computer_cross_rot(const size_t n,
                                                    const size_t d,
                                                    const size_t sub_vec_length,
                                                    const float signal_variance)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
  , signal_variance(signal_variance)
{
    const std::string name = "LLRs_computer_cross_rot";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d != 64)
    {
        std::stringstream message;
        message << "'d' has to be equal to 64 ('d' = " << d << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length * d != n)
    {
        std::stringstream message;
        message << "'sub_vec_length * d' has to be equal to 'n'\n('sub_vec_length * d' = " << sub_vec_length * d
                << " and 'n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (signal_variance < 1.0f)
    {
        std::stringstream message;
        message << "'signal_variance' has to be greater than or equal to 1.0 ('signal_variance' = " << signal_variance
                << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("compute_LLRs_cr");
    auto p1s_in_snr = this->template create_socket_in<float>(p1, "in_snr", 1);
    auto p1s_in_rotated_X = this->template create_socket_in<T>(p1, "in_rotated_X", this->n);
    auto p1s_in_Y_frob_norms = this->template create_socket_in<T>(p1, "in_Y_frob_norms", this->sub_vec_length);
    auto p1s_out_LLRs = this->template create_socket_out<float>(p1, "out_LLRs", this->n);
    this->create_codelet(p1,
                         [p1s_in_snr, p1s_in_rotated_X, p1s_in_Y_frob_norms, p1s_out_LLRs](
                           Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& LLRs_cr_cmp = static_cast<LLRs_computer_cross_rot&>(m);
                             LLRs_cr_cmp._compute_LLRs_cr(t[p1s_in_snr].template get_dataptr<const float>(),
                                                          t[p1s_in_rotated_X].template get_dataptr<const T>(),
                                                          t[p1s_in_Y_frob_norms].template get_dataptr<const T>(),
                                                          t[p1s_out_LLRs].template get_dataptr<float>(),
                                                          frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
LLRs_computer_cross_rot<T>*
LLRs_computer_cross_rot<T>::clone() const
{
    auto m = new LLRs_computer_cross_rot(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
LLRs_computer_cross_rot<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
LLRs_computer_cross_rot<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
LLRs_computer_cross_rot<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
float
LLRs_computer_cross_rot<T>::get_signal_variance() const
{
    return this->signal_variance;
}

template<typename T>
void
LLRs_computer_cross_rot<T>::compute_LLRs_cr(const float* in_snr,
                                            const T* in_rotated_X,
                                            const T* in_Y_frob_norms,
                                            float* out_LLRs,
                                            const int frame_id,
                                            const bool managed_memory)
{
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::in_snr].bind(in_snr);
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::in_rotated_X].bind(in_rotated_X);
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::in_Y_frob_norms].bind(in_Y_frob_norms);
    (*this)[LLR_cmp_cr::sck::compute_LLRs_cr::out_LLRs].bind(out_LLRs);
    (*this)[LLR_cmp_cr::tsk::compute_LLRs_cr].exec(frame_id, managed_memory);
}

template<typename T>
void
LLRs_computer_cross_rot<T>::_compute_LLRs_cr(const float* in_snr,
                                             const T* in_rotated_X,
                                             const T* in_Y_frob_norms,
                                             float* out_LLRs,
                                             const size_t /*frame_id*/)
{
    const T NOISE_VARIANCE = this->signal_variance / *in_snr;

    for (size_t i = 0; i < this->sub_vec_length; i++)
    {

        for (size_t j = 0; j < this->d; j++)
        {
            const T SCALE = (in_Y_frob_norms[i] * in_Y_frob_norms[i]) / (8 * std::sqrt(8));
            const T SIGNAL = in_rotated_X[i * this->d + j];
            out_LLRs[i * this->d + j] = SCALE * SIGNAL / NOISE_VARIANCE;
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::LLRs_computer_cross_rot<double>;
// ==================================================================================== explicit template instantiation
