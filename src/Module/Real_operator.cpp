#include <sstream>

#include "Module/Real_operator.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Real_operator<T>::Real_operator(const size_t n, const size_t d, const size_t sub_vec_length)
  : Stateful()
  , n(n)
  , d(d)
  , sub_vec_length(sub_vec_length)
{
    const std::string name = "Real_operator";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (d != 1)
    {
        std::stringstream message;
        message << "'d' should be equal to 1 ('d' = " << d << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (sub_vec_length != n)
    {
        std::stringstream message;
        message << "'sub_vec_length' has to be greater than 0 ('sub_vec_length' = " << sub_vec_length << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("map");
    auto p1s_in_U = this->template create_socket_in<float>(p1, "in_U", this->n);
    auto p1s_in_Y = this->template create_socket_in<T>(p1, "in_Y", this->n);
    auto p1s_out_mappings = this->template create_socket_out<T>(p1, "out_mappings", this->n);
    this->create_codelet(
      p1,
      [p1s_in_U, p1s_in_Y, p1s_out_mappings](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& rl_map = static_cast<Real_operator&>(m);
          rl_map._map(t[p1s_in_U].template get_dataptr<const float>(),
                      t[p1s_in_Y].template get_dataptr<const T>(),
                      t[p1s_out_mappings].template get_dataptr<T>(),
                      frame_id);
          return runtime::status_t::SUCCESS;
      });

    auto& p2 = this->create_task("demap");
    auto p2s_in_mappings = this->template create_socket_in<T>(p2, "in_mappings", this->n);
    auto p2s_in_X = this->template create_socket_in<T>(p2, "in_X", this->n);
    auto p2s_out_demappings = this->template create_socket_out<T>(p2, "out_demappings", this->n);
    this->create_codelet(
      p2,
      [p2s_in_mappings, p2s_in_X, p2s_out_demappings](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& rl_demap = static_cast<Real_operator&>(m);
          rl_demap._demap(t[p2s_in_mappings].template get_dataptr<const T>(),
                          t[p2s_in_X].template get_dataptr<const T>(),
                          t[p2s_out_demappings].template get_dataptr<T>(),
                          frame_id);
          return runtime::status_t::SUCCESS;
      });
}

template<typename T>
Real_operator<T>*
Real_operator<T>::clone() const
{
    auto m = new Real_operator(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
Real_operator<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Real_operator<T>::get_d() const
{
    return this->d;
}

template<typename T>
size_t
Real_operator<T>::get_sub_vec_length() const
{
    return this->sub_vec_length;
}

template<typename T>
void
Real_operator<T>::map(const float* in_U, const T* in_Y, T* out_mappings, const int frame_id, const bool managed_memory)
{
    (*this)[real_mapper::sck::map::in_U].bind(in_U);
    (*this)[real_mapper::sck::map::in_Y].bind(in_Y);
    (*this)[real_mapper::sck::map::out_mappings].bind(out_mappings);
    (*this)[real_mapper::tsk::map].exec(frame_id, managed_memory);
}

template<typename T>
void
Real_operator<T>::demap(const T* in_mappings,
                        const T* in_X,
                        T* out_demappings,
                        const int frame_id,
                        const bool managed_memory)
{
    (*this)[real_mapper::sck::demap::in_mappings].bind(in_mappings);
    (*this)[real_mapper::sck::demap::in_X].bind(in_X);
    (*this)[real_mapper::sck::demap::out_demappings].bind(out_demappings);
    (*this)[real_mapper::tsk::demap].exec(frame_id, managed_memory);
}

template<typename T>
void
Real_operator<T>::_map(const float* in_U, const T* in_Y, T* out_mappings, const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->n; i++)
    {
        out_mappings[i] = in_U[i] * in_Y[i];
    }
}

template<typename T>
void
Real_operator<T>::_demap(const T* in_mappings, const T* in_X, T* out_demappings, const size_t /*frame_id*/)
{
    for (size_t i = 0; i < this->n; i++)
    {
        out_demappings[i] = in_mappings[i] / in_X[i];
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Real_operator<double>;
// ==================================================================================== explicit template instantiation
