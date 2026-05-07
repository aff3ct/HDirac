#ifndef CODE_PARAMS_LDPC_HPP_
#define CODE_PARAMS_LDPC_HPP_

#include <iostream>

#include <aff3ct.hpp>

#include "Code_params.hpp"
#include "Tools/LDPC/LDPC_build_tools.hpp"

class CodeParamsLDPC : public CodeParams
{
  public:
    CodeParamsLDPC();
    CodeParamsLDPC(const std::string filename, const size_t n_ite, bool bsparse);
    CodeParamsLDPC(const std::string filename, const size_t n_ite, std::vector<uint32_t> bits_pos, bool bsparse);
    virtual ~CodeParamsLDPC() = default;

    size_t get_n_ite() const;
    const aff3ct::tools::Sparse_matrix& get_H() const;
    std::vector<uint32_t> get_bits_pos() const;

  protected:
    aff3ct::tools::Sparse_matrix _H; // H matrix
    size_t n_ite;                    // number of decoder iterations
    std::vector<uint32_t> bits_pos;  // information bits position for systematic codes
};

#endif
