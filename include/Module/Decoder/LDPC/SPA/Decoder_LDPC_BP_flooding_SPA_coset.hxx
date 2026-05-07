#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <limits>
#include <mipp.h>
#include <string>

#include "Module/Decoder/LDPC/SPA/Decoder_LDPC_BP_flooding_SPA_coset.hpp"

namespace CVQKD
{

namespace tools
{
void
LDPC_syndrome_coset::set_syndrome(const aff3ct::tools::Sparse_matrix& H, const int* syndrome)
{
    const aff3ct::tools::Sparse_matrix* H_ptr = &H;
    LDPC_syndrome_coset::H_to_syndrome[H_ptr] = syndrome;
}

template<typename R>
bool
LDPC_syndrome_coset::check_soft(const R* Y_N, const aff3ct::tools::Sparse_matrix& H)
{
    const aff3ct::tools::Sparse_matrix* H_ptr = &H;
    if (!LDPC_syndrome_coset::H_to_syndrome.count(H_ptr))
    {
        std::stringstream message;
        message << "The coset syndrome has not been set, please call LDPC_syndrome_coset::set_syndrome(...) first.";
        throw spu::tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    const int* syndrome_coset = LDPC_syndrome_coset::H_to_syndrome[H_ptr];

    auto syndrome = false;

    const auto n_chk_nodes = (int)H.get_n_cols();
    auto c = 0;
    while (c < n_chk_nodes && !syndrome)
    {
        auto sign = 0;

        const auto chk_degree = (int)H[c].size();
        for (auto v = 0; v < chk_degree; v++)
        {
            const auto llr = Y_N[H[c][v]];
            const auto tmp_sign = (llr < 0) ? -1 : 0;

            sign ^= tmp_sign;
        }

        int coset = syndrome_coset[c] ? -1 : 0;
        syndrome = syndrome || (sign ^ coset);
        c++;
    }

    return !syndrome;
}

template<typename R>
bool
LDPC_syndrome_coset::check_soft(const std::vector<R>& Y_N, const aff3ct::tools::Sparse_matrix& H)
{
    return LDPC_syndrome_coset::check_soft<R>(Y_N.data(), H);
}

}

namespace module
{

template<typename B, typename R>
runtime::Task&
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::operator[](const aff3ct::module::dec::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename B, typename R>
runtime::Task&
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::operator[](const CVQKD::module::dec::tsk t)
{
    constexpr size_t base_size = (size_t)aff3ct::module::dec::tsk::SIZE;
    return Module::operator[](base_size + (size_t)t);
}

template<typename B, typename R>
runtime::Socket&
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::operator[](const CVQKD::module::dec::sck::set_syndrome s)
{
    return Module::operator[]((size_t)CVQKD::module::dec::tsk::set_syndrome)[(size_t)s];
}

template<typename B, typename R>
runtime::Socket&
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename B, typename R>
template<class A>
void
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::set_syndrome(const std::vector<int, A>& in_syndrome,
                                                       const int frame_id,
                                                       const bool managed_memory)
{
    (*this)[CVQKD::module::dec::sck::set_syndrome::in_syndrome].bind(in_syndrome);
    (*this)[CVQKD::module::dec::tsk::set_syndrome].exec(frame_id, managed_memory);
}

}
}
