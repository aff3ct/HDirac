#include <sstream>

#include "Module/Quantum_channel.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Quantum_channel<T>::Quantum_channel(const size_t n,
                                    aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen,
                                    const float signal_variance)
  : Stateful()
  , n(n)
  , normal_gen(normal_gen.clone())
  , signal_variance(signal_variance)
{
    const std::string name = "Quantum_channel";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (signal_variance < 1.0f)
    {
        std::stringstream message;
        message << "'signal_variance' has to be greater than or equal to 1.0 ('signal_variance' = " << signal_variance
                << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("add_noise");
    auto p1s_in_snr = this->template create_socket_in<float>(p1, "in_snr", 1);
    auto p1s_in_X = this->template create_socket_in<T>(p1, "in_X", this->n);
    auto p1s_out_Y = this->template create_socket_out<T>(p1, "out_Y", this->n);
    this->create_codelet(p1,
                         [p1s_in_snr, p1s_in_X, p1s_out_Y](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& q_chn = static_cast<Quantum_channel&>(m);
                             q_chn._add_noise(t[p1s_in_snr].template get_dataptr<const float>(),
                                              t[p1s_in_X].template get_dataptr<const T>(),
                                              t[p1s_out_Y].template get_dataptr<T>(),
                                              frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Quantum_channel<T>*
Quantum_channel<T>::clone() const
{
    auto m = new Quantum_channel(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
void
Quantum_channel<T>::set_seed(const int seed)
{
    this->normal_gen->set_seed(seed);
}

template<typename T>
size_t
Quantum_channel<T>::get_n() const
{
    return this->n;
}

template<typename T>
float
Quantum_channel<T>::get_signal_variance() const
{
    return this->signal_variance;
}

template<typename T>
void
Quantum_channel<T>::deep_copy(const Quantum_channel<T>& m)
{
    spu::module::Stateful::deep_copy(m);
    if (m.normal_gen != nullptr) this->normal_gen.reset(m.normal_gen->clone());
}

template<typename T>
void
Quantum_channel<T>::add_noise(const float* in_snr,
                              const T* in_X,
                              T* out_Y,
                              const int frame_id,
                              const bool managed_memory)
{
    (*this)[q_channel::sck::add_noise::in_snr].bind(in_snr);
    (*this)[q_channel::sck::add_noise::in_X].bind(in_X);
    (*this)[q_channel::sck::add_noise::out_Y].bind(out_Y);
    (*this)[q_channel::tsk::add_noise].exec(frame_id, managed_memory);
}

template<typename T>
void
Quantum_channel<T>::_add_noise(const float* in_snr, const T* in_X, T* out_Y, const size_t /*frame_id*/)
{
    // Normalised signal power variance
    const float CHANNEL_STD = sqrt(this->signal_variance / *in_snr);

    // Generate Gaussian noise
    this->normal_gen->generate(out_Y, n, CHANNEL_STD, 0.0);

    for (size_t i = 0; i < n; i++)
    {
        // Gaussian noise addition to the signal
        out_Y[i] += in_X[i];
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Quantum_channel<float>;
template class CVQKD::module::Quantum_channel<double>;
// ==================================================================================== explicit template instantiation
