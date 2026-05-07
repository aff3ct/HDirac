#include <sstream>

#include "Module/Decoder/LDPC/Syndrome_computer.hpp"

using namespace CVQKD;
using namespace CVQKD::module;

template<typename T>
Syndrome_computer<T>::Syndrome_computer(const size_t n, const size_t r, const aff3ct::tools::Sparse_matrix& _H)
  : Stateful()
  , n(n)
  , r(r)
  , _H(_H)
{
    const std::string name = "Syndrome_computer";
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
        message << "'r' has to be grater than 0 ('r' = " << r << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("compute_syndrome");
    auto p1s_in_codeword = this->template create_socket_in<T>(p1, "in_codeword", this->n);
    auto p1s_out_syndrome = this->template create_socket_out<T>(p1, "out_syndrome", this->r);
    this->create_codelet(p1,
                         [p1s_in_codeword, p1s_out_syndrome](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& cmp_synd = static_cast<Syndrome_computer&>(m);
                             cmp_synd._compute_syndrome(t[p1s_in_codeword].template get_dataptr<const T>(),
                                                        t[p1s_out_syndrome].template get_dataptr<T>(),
                                                        frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Syndrome_computer<T>*
Syndrome_computer<T>::clone() const
{
    auto m = new Syndrome_computer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
Syndrome_computer<T>::get_n() const
{
    return this->n;
}

template<typename T>
size_t
Syndrome_computer<T>::get_r() const
{
    return this->r;
}

template<typename T>
void
Syndrome_computer<T>::compute_syndrome(const T* in_codeword,
                                       const T* out_syndrome,
                                       const int frame_id,
                                       const bool managed_memory)
{
    (*this)[synd_cmp::sck::compute_syndrome::in_codeword].bind(in_codeword);
    (*this)[synd_cmp::sck::compute_syndrome::out_syndrome].bind(out_syndrome);
    (*this)[synd_cmp::tsk::compute_syndrome].exec(frame_id, managed_memory);
}

template<typename T>
void
Syndrome_computer<T>::_compute_syndrome(const T* in_codeword, T* out_syndrome, const size_t /*frame_id*/)
{
    int nrows = this->_H.get_n_cols();
    for (int i = 0; i < nrows; i++)
    {
        out_syndrome[i] = 0;
        auto cols = this->_H.get_rows_from_col(i);
        for (auto j : cols)
        {
            out_syndrome[i] ^= in_codeword[j];
        }
    }
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Syndrome_computer<int>;
// ==================================================================================== explicit template instantiation
