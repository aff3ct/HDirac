#include "Code/Code_params_LDPC.hpp"
#include "Tools/LDPC/LDPC_build_tools.hpp"

CodeParamsLDPC::CodeParamsLDPC() {}

CodeParamsLDPC::CodeParamsLDPC(const std::string filename, const size_t n_ite, bool bsparse)
{
    if (bsparse)
    {
        this->_H = CVQKD::tools::bsparse_read(filename);
    }
    else
    {
        this->_H = CVQKD::tools::load_from_alist(filename);
    }

    this->n = this->_H.get_n_rows();
    this->r = this->_H.get_n_cols();
    this->k = this->n - this->r;
    this->R = (float)this->k / (float)this->n;
    this->n_ite = n_ite;
}

CodeParamsLDPC::CodeParamsLDPC(std::string filename, size_t n_ite, std::vector<uint32_t> bits_pos, bool bsparse)
{
    CodeParamsLDPC(filename, n_ite, bsparse);
    this->bits_pos = bits_pos;
}

size_t
CodeParamsLDPC::get_n_ite() const
{
    return this->n_ite;
}

const aff3ct::tools::Sparse_matrix&
CodeParamsLDPC::get_H() const
{
    return this->_H;
}

std::vector<uint32_t>
CodeParamsLDPC::get_bits_pos() const
{
    return this->bits_pos;
}