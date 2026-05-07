#include "Code/Code_params_LDPC_synd_cnct.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

CodeParamsLDPCSyndCnct::CodeParamsLDPCSyndCnct() {}

CodeParamsLDPCSyndCnct::CodeParamsLDPCSyndCnct(const std::string filename, const size_t n_ite, bool bsparse)
  : CodeParamsLDPC(filename, n_ite, bsparse)
{
    this->n_orig = this->n;
    this->r_orig = this->r;
    this->k_orig = this->k;

    this->n = this->n_orig + this->r_orig;
    this->r = this->r_orig;
    this->k = this->n_orig;
    this->bits_pos = std::vector<uint32_t>(this->k);

    this->_P = this->_H;
    this->_H = this->_H.resize(this->n, this->r, aff3ct::tools::Sparse_matrix::Origin::TOP_LEFT);

    for (size_t i = 0; i < this->r; i++)
    {
        this->_H.add_connection(this->n_orig + i, i);
    }
    iota(this->bits_pos.begin(), this->bits_pos.end(), 0);
}

void
CodeParamsLDPCSyndCnct::compute_syndrome_non_systematic(const std::vector<int>& ref_bits, std::vector<int>& syndrome)
{
    int nrows = this->_P.get_n_cols();
    for (int i = 0; i < nrows; i++)
    {
        syndrome[i] = 0;
        auto cols = this->_P.get_rows_from_col(i);
        for (auto j : cols)
        {
            syndrome[i] ^= ref_bits[j];
        }
    }
}

size_t
CodeParamsLDPCSyndCnct::get_k_orig() const
{
    return this->k_orig;
}

size_t
CodeParamsLDPCSyndCnct::get_n_orig() const
{
    return this->n_orig;
}

size_t
CodeParamsLDPCSyndCnct::get_r_orig() const
{
    return this->r_orig;
}

const aff3ct::tools::Sparse_matrix&
CodeParamsLDPCSyndCnct::get_P() const
{
    return this->_P;
}