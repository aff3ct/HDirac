#include <sstream>

#include <aff3ct.hpp>

#include "Module/LLRs_formatter.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
LLRs_formatter<T>::LLRs_formatter(const size_t n, const size_t r, const aff3ct::tools::Sparse_matrix& _P)
  : Stateful()
  , n(n)
  , r(r)
  , fmt_n(n + r)
  , _P(_P)
{
    const std::string name = "LLRs_formatter";
    this->set_name(name);
    this->set_short_name(name);

    if (n == 0)
    {
        std::stringstream message;
        message << "'n' has to be greater than 0 ('n' = " << n << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (r == 0)
    {
        std::stringstream message;
        message << "'r' has to be greater than 0 ('r' = " << r << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->syndrome = new int[this->r];

    auto& p1 = this->create_task("format_LLRs");
    auto p1s_in_U_ref = this->template create_socket_in<int>(p1, "in_U_ref", this->n);
    auto p1s_in_LLRs = this->template create_socket_in<T>(p1, "in_LLRs", this->n);
    auto p1s_out_fmt_LLRs = this->template create_socket_out<T>(p1, "out_fmt_LLRs", this->fmt_n);
    this->create_codelet(
      p1,
      [p1s_in_U_ref, p1s_in_LLRs, p1s_out_fmt_LLRs](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& LLRs_fmt = static_cast<LLRs_formatter&>(m);
          LLRs_fmt._format_LLRs(t[p1s_in_U_ref].template get_dataptr<const int>(),
                                t[p1s_in_LLRs].template get_dataptr<const T>(),
                                t[p1s_out_fmt_LLRs].template get_dataptr<T>(),
                                frame_id);
          return runtime::status_t::SUCCESS;
      });
}

template<typename T>
LLRs_formatter<T>::~LLRs_formatter()
{
    delete[] this->syndrome;
}

template<typename T>
LLRs_formatter<T>*
LLRs_formatter<T>::clone() const
{
    auto m = new LLRs_formatter(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
void
LLRs_formatter<T>::deep_copy(const LLRs_formatter<T>& m)
{
    spu::module::Stateful::deep_copy(m);

    this->syndrome = new int[this->r];

    if (m.syndrome != nullptr)
    {
        std::copy(m.syndrome, m.syndrome + this->r, this->syndrome);
    }
}

template<typename T>
size_t
LLRs_formatter<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
LLRs_formatter<T>::get_r() const
{
    return this->r;
}

template<typename T>
size_t
LLRs_formatter<T>::get_fmt_n() const
{
    return this->fmt_n;
}

template<typename T>
void
LLRs_formatter<T>::format_LLRs(const int* in_U_ref,
                               const T* in_LLRs,
                               T* out_fmt_LLRs,
                               const int frame_id,
                               const bool managed_memory)
{
    (*this)[LLRs_fmt::sck::format_LLRs::in_U_ref].bind(in_U_ref);
    (*this)[LLRs_fmt::sck::format_LLRs::in_LLRs].bind(in_LLRs);
    (*this)[LLRs_fmt::sck::format_LLRs::out_fmt_LLRs].bind(out_fmt_LLRs);
    (*this)[LLRs_fmt::tsk::format_LLRs].exec(frame_id, managed_memory);
}

template<typename T>
void
LLRs_formatter<T>::compute_syndrome(const int* ref_bits)
{
    int nrows = this->_P.get_n_cols();
    for (int i = 0; i < nrows; i++)
    {
        this->syndrome[i] = 0;
        auto cols = this->_P.get_rows_from_col(i);
        for (auto j : cols)
        {
            this->syndrome[i] ^= ref_bits[j];
        }
    }
}

template<typename T>
void
LLRs_formatter<T>::_format_LLRs(const int* in_U_ref, const T* in_LLRs, T* out_fmt_LLRs, const size_t frame_id)
{
    this->compute_syndrome(in_U_ref);

    for (size_t i = 0; i < this->fmt_n; i++)
    {
        if (i < this->n)
        {
            out_fmt_LLRs[i] = in_LLRs[i];
        }
        else
        {
            out_fmt_LLRs[i] = (this->syndrome[i - this->n] % 2 == 0) ? INFINITY : -INFINITY;
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::LLRs_formatter<float>;
// ==================================================================================== explicit template instantiation
