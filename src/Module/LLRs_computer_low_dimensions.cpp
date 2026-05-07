
#include <sstream>

#include "Module/LLRs_computer_low_dimensions.hpp"
#include "Tools/Helper.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
LLRs_computer_low_dimensions<T>::LLRs_computer_low_dimensions(const size_t n,
                                                              const size_t d,
                                                              const size_t sub_vec_length,
                                                              const float signal_variance)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
  , signal_variance(signal_variance)
{
    const std::string name = "LLRs_computer_low_dimensions";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d == 0)
    {
        std::stringstream message;
        message << "'d' has to be greater than 0 ('d' = " << d << ").";
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

    auto& p1 = this->create_task("compute_LLRs_low_d");
    auto p1s_in_snr = this->template create_socket_in<float>(p1, "in_snr", 1);
    auto p1s_in_X = this->template create_socket_in<T>(p1, "in_X", this->n);
    auto p1s_in_quotients = this->template create_socket_in<T>(p1, "in_quotients", this->n);
    auto p1s_out_LLRs = this->template create_socket_out<float>(p1, "out_LLRs", this->n);
    this->create_codelet(
      p1,
      [p1s_in_snr, p1s_in_X, p1s_in_quotients, p1s_out_LLRs](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& LLRs_cmp = static_cast<LLRs_computer_low_dimensions&>(m);
          LLRs_cmp._compute_LLRs_low_d(t[p1s_in_snr].template get_dataptr<const float>(),
                                       t[p1s_in_X].template get_dataptr<const T>(),
                                       t[p1s_in_quotients].template get_dataptr<const T>(),
                                       t[p1s_out_LLRs].template get_dataptr<float>(),
                                       frame_id);
          return runtime::status_t::SUCCESS;
      });
}

template<typename T>
LLRs_computer_low_dimensions<T>*
LLRs_computer_low_dimensions<T>::clone() const
{
    auto m = new LLRs_computer_low_dimensions(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
LLRs_computer_low_dimensions<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
LLRs_computer_low_dimensions<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
LLRs_computer_low_dimensions<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
float
LLRs_computer_low_dimensions<T>::get_signal_variance() const
{
    return this->signal_variance;
}

template<typename T>
void
LLRs_computer_low_dimensions<T>::compute_LLRs_low_d(const float* in_snr,
                                                    const T* in_X,
                                                    const T* in_quotients,
                                                    float* out_LLRs,
                                                    const int frame_id,
                                                    const bool managed_memory)
{
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::in_snr].bind(in_snr);
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::in_X].bind(in_X);
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::in_quotients].bind(in_quotients);
    (*this)[LLR_cmp_low_d::sck::compute_LLRs_low_d::out_LLRs].bind(out_LLRs);
    (*this)[LLR_cmp_low_d::tsk::compute_LLRs_low_d].exec(frame_id, managed_memory);
}

template<typename T>
void
LLRs_computer_low_dimensions<T>::_compute_LLRs_low_d(const float* in_snr,
                                                     const T* in_X,
                                                     const T* in_quotients,
                                                     float* out_LLRs,
                                                     const size_t /*frame_id*/)
{
    float scalar_mul = 0.0f;

    for (size_t i = 0; i < this->sub_vec_length; i++)
    {
        scalar_mul =
          2 * *in_snr * CVQKD::tools::compute_norm2(&in_X[i * this->d], this->d) / (d * this->signal_variance);

        for (size_t j = 0; j < this->d; j++)
        {

            out_LLRs[i * this->d + j] = scalar_mul * static_cast<float>(in_quotients[i * this->d + j]);
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::LLRs_computer_low_dimensions<double>;
// ==================================================================================== explicit template instantiation
