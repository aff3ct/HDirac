#ifndef CODE_PARAMS_SYND_CNCT_LDPC_HPP_
#define CODE_PARAMS_SYND_CNCT_LDPC_HPP_

#include <aff3ct.hpp>

#include "Code_params_LDPC.hpp"

class CodeParamsLDPCSyndCnct : public CodeParamsLDPC
{
  public:
    CodeParamsLDPCSyndCnct();
    CodeParamsLDPCSyndCnct(const std::string filename, const size_t n_ite, bool bsparse);
    void compute_syndrome_non_systematic(const std::vector<int>& ref_bits, std::vector<int>& syndrome);
    size_t get_k_orig() const;
    size_t get_n_orig() const;
    size_t get_r_orig() const;
    const aff3ct::tools::Sparse_matrix& get_P() const;

  private:
    aff3ct::tools::Sparse_matrix _P; // actual H matrix
    size_t k_orig;                   // number of information bits
    size_t n_orig;                   // codeword size
    size_t r_orig;                   // syndrome size
};

#endif