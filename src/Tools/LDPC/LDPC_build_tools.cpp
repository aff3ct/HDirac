#include "Tools/LDPC/LDPC_build_tools.hpp"

#include <iostream>

aff3ct::tools::Sparse_matrix
CVQKD::tools::load_from_alist(std::string filepath)
{
    std::cerr << "Reading alist file...  ";
    std::ifstream file;
    file.open(filepath, std::ios::in);
    if (file.fail())
    {
        std::cerr << "Error on file opening for load_from_alist(). Filename: " << filepath << std::endl;
        throw std::exception();
    }

    aff3ct::tools::Sparse_matrix H = aff3ct::tools::AList::read(file);
    file.close();
    std::cerr << "...done." << std::endl;
    return H;
}

CVQKD::tools::temporary_code
CVQKD::tools::new_from_cil(std::string path, std::string filename)
{
    CVQKD::tools::temporary_code H;

    // get max_cn_deg from CN_degrees file
    std::string cn_path = path + "CN_degrees_" + filename + ".txt";
    std::ifstream cn_file(cn_path);
    if (!cn_file.is_open())
    {
        std::cerr << "Error: Could not open " << cn_path << std::endl;
        throw std::runtime_error("File error");
    }

    size_t max_cn_deg;
    cn_file >> max_cn_deg;
    cn_file.close();

    // get n and connections from VN_connections file
    std::string vn_path = path + "VN_connections_" + filename + ".txt";
    std::ifstream vn_file(vn_path);
    if (!vn_file.is_open())
    {
        std::cerr << "Error: Could not open " << vn_path << std::endl;
        throw std::runtime_error("File error");
    }

    vn_file >> H.n;

    size_t val;
    while (vn_file >> val)
    {
        std::vector<size_t> current_row;
        if (val > 0) current_row.push_back(val - 1);

        for (size_t i = 1; i < max_cn_deg; i++)
        {
            if (vn_file >> val)
            {
                if (val > 0) current_row.push_back(val - 1);
            }
        }
        H.rows.push_back(current_row);
    }

    vn_file.close();
    return H;
}

void
CVQKD::tools::augment_rate(temporary_code& H, float new_rate)
{
    // k must be fixed (20000)
    const size_t k = 20000;
    H.n = static_cast<size_t>(static_cast<float>(k) / new_rate);
    size_t r = H.n - k;

    std::cout << "n: " << H.n << ", r: " << r << ", rate: " << (float)k / H.n << std::endl;

    if (r < H.rows.size())
    {
        H.rows.resize(r);
    }
    else
    {
        H.rows.resize(r, std::vector<size_t>());
    }
}

void
CVQKD::tools::make_qc(temporary_code& H, size_t q)
{
    size_t old_r = H.rows.size();
    size_t new_r = old_r * q;
    size_t new_n = H.n * q;

    std::vector<std::vector<size_t>> new_rows(new_r);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, q - 1);

    for (size_t i = 0; i < old_r; ++i)
    {
        for (size_t j : H.rows[i])
        {
            // Random cyclic shift for this specific edge
            size_t shift = dist(gen);

            // Expand the edge into a qxq permutation matrix
            for (size_t k = 0; k < q; k++)
            {
                size_t row_idx = i * q + k;
                size_t col_idx = j * q + (k + shift) % q;
                new_rows[row_idx].push_back(col_idx);
            }
        }
    }

    // Update the matrix in-place
    H.rows = std::move(new_rows);
    H.n = new_n;
}

void
CVQKD::tools::bsparse_write(temporary_code& H, std::string bsparse_matrix_path, std::string cil_matrix_filename)
{
    // Construct the full path
    // This assumes bsparse_matrix_path already ends with a '/' or is just a prefix
    std::string full_path = bsparse_matrix_path + cil_matrix_filename;

    std::ofstream file(full_path, std::ios::out | std::ios::binary);
    if (file.fail())
    {
        std::cerr << "Error on file opening for bsparse_write(). Filename: " << full_path << std::endl;
        throw std::runtime_error("Could not open bsparse file for writing");
    }

    // Header : number of VN, number of CN
    // Cast to uint32_t to match the 4-byte requirement of the format
    uint32_t N = static_cast<uint32_t>(H.n);
    uint32_t r = static_cast<uint32_t>(H.rows.size());

    file.write(reinterpret_cast<const char*>(&N), 4);
    file.write(reinterpret_cast<const char*>(&r), 4);

    // Row index i corresponds to the Check Node
    // Values in H.rows[i] correspond to Variable Node indices
    for (uint32_t i = 0; i < r; ++i)
    {
        for (size_t col_idx : H.rows[i])
        {
            uint32_t j = static_cast<uint32_t>(col_idx);
            uint32_t check_node_idx = i;

            // Following the format: variable index followed by check node index
            file.write(reinterpret_cast<const char*>(&j), 4);
            file.write(reinterpret_cast<const char*>(&check_node_idx), 4);
        }
    }

    file.close();
    std::cerr << "Successfully wrote bsparse matrix to: " << full_path << std::endl;
}

/**
 * bsparse format:
 * Sequence of pairs of uint32 integers
 * (The total file size is a multiple of 8 bytes)
 * First pair is the matrix size:
 *   number of variable nodes followed by number of check nodes
 * Following pairs are indices of edges in the graph:
 *   variable index followed by check node index
 *
 */
aff3ct::tools::Sparse_matrix
CVQKD::tools::bsparse_read(std::string filename)
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (file.fail())
    {
        std::cerr << "Error on file opening for bsparse_read(). Filename: " << filename << std::endl;
        throw std::exception();
    }

    uint32_t N;
    uint32_t r;
    file.read((char*)&N, 4);
    file.read((char*)&r, 4);
    auto H = aff3ct::tools::Sparse_matrix(N, r);

    while (true)
    {
        uint32_t i;
        uint32_t j;
        file.read((char*)&i, 4);
        file.read((char*)&j, 4);
        if (file.eof())
        {
            break;
        }
        H.add_connection(i, j);
    }

    file.close();
    return H;
}
