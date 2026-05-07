// Build LDPC parity check matrices.

#ifndef LDPC_BUILD_TOOLS_HPP_
#define LDPC_BUILD_TOOLS_HPP_

#include <string>
#include <vector>

#include <aff3ct.hpp>

namespace CVQKD
{
namespace tools
{

struct temporary_code
{
    std::vector<std::vector<size_t>> rows;
    size_t n;
};

// Load a sparse martix from an alist file
aff3ct::tools::Sparse_matrix
load_from_alist(std::string filepath);

// Write a sparse matrix to file in bsparse format
void
bsparse_write(temporary_code& H, std::string bsparse_matrix_path, std::string cil_matrix_filename);

// Load a Cil matrix into a
temporary_code
new_from_cil(std::string path, std::string filename);

void
augment_rate(temporary_code& H, float new_rate);

void
make_qc(temporary_code& H, size_t q);

// Read a sparse matrix from file in bsparse format
aff3ct::tools::Sparse_matrix
bsparse_read(std::string filename);

}
}

#endif
