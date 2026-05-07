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
std::map<const aff3ct::tools::Sparse_matrix*, const int*> LDPC_syndrome_coset::H_to_syndrome;
}

namespace module
{

template<typename B, typename R>
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::Decoder_LDPC_BP_flooding_SPA_coset(const int K,
                                                                             const int N,
                                                                             const int r,
                                                                             const int n_ite,
                                                                             const aff3ct::tools::Sparse_matrix& _H,
                                                                             const std::vector<uint32_t>& info_bits_pos,
                                                                             const bool enable_syndrome,
                                                                             const int syndrome_depth)
  : aff3ct::module::Decoder_LDPC_BP_flooding<B, R, aff3ct::tools::Update_rule_SPA<R>, tools::LDPC_syndrome_coset>(
      K,
      N,
      n_ite,
      _H,
      info_bits_pos,
      aff3ct::tools::Update_rule_SPA<R>((unsigned int)_H.get_cols_max_degree()),
      enable_syndrome,
      syndrome_depth)
  , values(_H.get_cols_max_degree())
  , r(r)
// r can't be computed as n - k since for this coset decoder n and k are equal because they represent resp. the
// number of input and output bits of the decoder
{
    const std::string name = "Decoder_LDPC_BP_flooding_SPA_coset";
    this->set_name(name);
    for (auto& t : this->tasks)
        t->set_replicability(true);

    fwd.resize(this->H.get_cols_max_degree());
    bwd.resize(this->H.get_cols_max_degree());

    auto& p1 = this->create_task("set_syndrome");
    auto p1s_in_syndrome = this->template create_socket_in<int>(p1, "in_syndrome", this->r);
    this->create_codelet(p1,
                         [p1s_in_syndrome](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& set_sdr = static_cast<Decoder_LDPC_BP_flooding_SPA_coset&>(m);
                             set_sdr._set_syndrome(t[p1s_in_syndrome].template get_dataptr<const int>(), frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename B, typename R>
Decoder_LDPC_BP_flooding_SPA_coset<B, R>*
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::clone() const
{
    auto m = new Decoder_LDPC_BP_flooding_SPA_coset(*this);
    m->deep_copy(*this);
    return m;
}

template<typename B, typename R>
void
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::_decode_single_ite(const std::vector<R>& msg_var_to_chk,
                                                             std::vector<R>& msg_chk_to_var)
{
    const auto n_branches = (int)this->H.get_n_connections();
    auto transpose_ptr = this->transpose.data();

    const R max_val = (R)1.0 - (R)1e-7;
    const auto vec_size = mipp::N<R>();
    const auto vec_loop_size = (n_branches / vec_size) * vec_size;

    const mipp::Reg<R> r_half = (R)0.5;
    const mipp::Reg<R> r_max = (R)max_val;
    const mipp::Reg<R> r_min = (R)(-max_val);

    for (auto b = 0; b < vec_loop_size; b += vec_size)
    {
        mipp::Reg<R> r = &msg_var_to_chk[b];

        r = mipp::tanh(r_half * r);

        // avoid abs(r) = 1 so we don't get +-inf
        r = mipp::min(mipp::max(r, r_min), r_max);

        r.storeu(&msg_chk_to_var[b]);
    }

    // tail
    for (auto b = vec_loop_size; b < n_branches; b++)
    {
        R t = std::tanh((R)0.5 * msg_var_to_chk[b]);
        msg_chk_to_var[b] = (t > max_val) ? max_val : (t < -max_val ? -max_val : t);
    }

    const auto n_chk_nodes = (int)this->H.get_n_cols();

    for (auto c = 0; c < n_chk_nodes; c++)
    {
        const int chk_degree = (int)this->H.get_col_to_rows()[c].size();

        R* f = fwd.data();
        R* b = bwd.data();

        f[0] = msg_chk_to_var[transpose_ptr[0]];
        for (int v = 1; v < chk_degree; v++)
            f[v] = f[v - 1] * msg_chk_to_var[transpose_ptr[v]];

        b[chk_degree - 1] = msg_chk_to_var[transpose_ptr[chk_degree - 1]];

        for (int v = chk_degree - 2; v >= 0; v--)
            b[v] = b[v + 1] * msg_chk_to_var[transpose_ptr[v]];

        const R syndrome_fix = (this->syndrome[c] == 1) ? (R)-1 : (R)1;

        for (int v = 0; v < chk_degree; v++)
        {
            R res;

            // using fw pass times bw pass to avoid division
            if (v == 0)
            {
                res = b[1];
            }
            else if (v == chk_degree - 1)
            {
                res = f[chk_degree - 2];
            }
            else
            {
                res = f[v - 1] * b[v + 1];
            }

            msg_chk_to_var[transpose_ptr[v]] = res * syndrome_fix;
        }

        transpose_ptr += chk_degree;
    }

    for (auto b = 0; b < vec_loop_size; b += vec_size)
    {
        mipp::Reg<R> r = &msg_chk_to_var[b];
        r = mipp::atanh(r) * (R)2.0;
        r.storeu(&msg_chk_to_var[b]);
    }

    // tail
    for (auto b = vec_loop_size; b < n_branches; b++)
    {
        msg_chk_to_var[b] = (R)2.0 * std::atanh(msg_chk_to_var[b]);
    }
}

template<typename B, typename R>
void
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::set_syndrome(const int* in_syndrome,
                                                       const int frame_id,
                                                       const bool managed_memory)
{
    (*this)[dec::sck::set_syndrome::in_syndrome].bind(in_syndrome);
    (*this)[dec::tsk::set_syndrome].exec(frame_id, managed_memory);
}

template<typename B, typename R>
void
Decoder_LDPC_BP_flooding_SPA_coset<B, R>::_set_syndrome(const int* in_syndrome, const size_t /*frame_id*/)
{
    this->syndrome = in_syndrome;
    tools::LDPC_syndrome_coset::set_syndrome(this->H, this->syndrome);
}

}
}

// ==================================================================================== explicit template instantiation
template class CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<int, float>;
// ==================================================================================== explicit template instantiation