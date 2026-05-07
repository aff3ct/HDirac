#include <sstream>

#include "Module/Alice_quantum_generator.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Alice_quantum_generator<T>::Alice_quantum_generator(const size_t n,
                                                    const float alice_var,
                                                    aff3ct::tools::Gaussian_noise_generator_std<T>& normal_gen)
  : Stateful()
  , alice_var(alice_var)
  , n(n)
  , normal_gen(normal_gen.clone())
{
    const std::string name = "Alice_quantum_generator";
    this->set_name(name);
    this->set_short_name(name);

    if (alice_var == 0)
    {
        std::stringstream message;
        message << "'alice_var' has to be greater than 0 ('alice_var' = " << alice_var << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("generate");
    auto p1s_out_X = this->template create_socket_out<T>(p1, "out_X", this->n);
    this->create_codelet(p1,
                         [p1s_out_X](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& gen = static_cast<Alice_quantum_generator&>(m);
                             gen._generate(t[p1s_out_X].template get_dataptr<T>(), frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Alice_quantum_generator<T>*
Alice_quantum_generator<T>::clone() const
{
    auto m = new Alice_quantum_generator(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
void
Alice_quantum_generator<T>::set_seed(const int seed)
{
    this->normal_gen->set_seed(seed);
}

template<typename T>
float
Alice_quantum_generator<T>::get_alice_var() const
{
    return this->alice_var;
}

template<typename T>
size_t
Alice_quantum_generator<T>::get_n() const
{
    return this->n;
}

template<typename T>
void
Alice_quantum_generator<T>::deep_copy(const Alice_quantum_generator<T>& m)
{
    spu::module::Stateful::deep_copy(m);
    if (m.normal_gen != nullptr) this->normal_gen.reset(m.normal_gen->clone());
}

template<typename T>
void
Alice_quantum_generator<T>::generate(T* out_X, const int frame_id, const bool managed_memory)
{
    (*this)[gen::sck::generate::out_X].bind(out_X);
    (*this)[gen::tsk::generate].exec(frame_id, managed_memory);
}

template<typename T>
void
Alice_quantum_generator<T>::_generate(T* out_X, const size_t /*frame_id*/)
{
    this->normal_gen->generate(out_X, n, std::sqrt(alice_var), 0.0);
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Alice_quantum_generator<float>;
template class CVQKD::module::Alice_quantum_generator<double>;
// ==================================================================================== explicit template instantiation
