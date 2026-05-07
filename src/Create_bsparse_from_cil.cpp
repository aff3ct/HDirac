#include "Tools/LDPC/LDPC_build_tools.hpp"

#define QC_EXAMPLE

void
display_help(const char* prog_name)
{
    std::cerr << "====================================================\n";
    std::cerr << "   LDPC Matrix Build Tool - CVQKD\n";
    std::cerr << "====================================================\n";
    std::cerr << "Usage: " << prog_name << " [OPTIONS]\n\n";
    std::cerr << "Required Arguments:\n";
    std::cerr << "  -p <path>    Path to the input matrix file\n";
    std::cerr << "  -f <path>    Name of the matrix file (e.g. R_0p2_R_0p01_RA_SPPCOM_2)\n";
    std::cerr << "  -r <float>   Target code rate\n";
    std::cerr << "  -o <path>    Path to the output .bsparse file\n";
    std::cerr << "  -n <path>    Name of the .bsparse file\n\n";
    std::cerr << "Optional Arguments:\n";
    std::cerr << "  -e <size_t>  Expansion factor (optional)\n";
    std::cerr << "  -h           Display help\n";
    std::cerr << "====================================================\n";
}

void
parse_args(int argc,
           char** argv,
           std::string& cil_matrix_path,
           std::string& cil_matrix_filename,
           float& rate,
           size_t& exp_factor,
           std::string& bsparse_matrix_path,
           std::string& bsparse_matrix_filename)
{
    int opt;
    bool p_set = false, f_set = false, r_set = false, o_set = false, n_set = false;

    // Set defaults
    exp_factor = 1;

    // p: path, f: filename, r: rate, o: out_path, n: out_name, e: expansion, h: help
    while ((opt = getopt(argc, argv, "p:f:r:o:n:e:h")) != -1)
    {
        switch (opt)
        {
            case 'p':
            {

                cil_matrix_path = optarg;
                p_set = true;
                break;
            }

            case 'f':
            {

                cil_matrix_filename = optarg;
                f_set = true;
                break;
            }

            case 'r':
            {
                try
                {
                    rate = std::stof(optarg);
                    r_set = true;
                }
                catch (...)
                {
                    std::cerr << "Error: Invalid rate value '" << optarg << "'\n";
                    exit(EXIT_FAILURE);
                }
                break;
            }

            case 'o':
            {

                bsparse_matrix_path = optarg;
                o_set = true;
                break;
            }

            case 'n':
            {

                bsparse_matrix_filename = optarg;
                n_set = true;
                break;
            }

            case 'e':
            {
                try
                {
                    exp_factor = std::stoul(optarg);
                }
                catch (...)
                {
                    std::cerr << "Error: Invalid expansion factor '" << optarg << "'\n";
                    exit(EXIT_FAILURE);
                }
                break;
            }

            case 'h':
            {
                display_help(argv[0]);
                exit(EXIT_SUCCESS);
            }

            default:
            {
                display_help(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Check required arguments
    if (!p_set || !f_set || !r_set || !o_set || !n_set)
    {
        std::cerr << "Error: Missing required arguments!\n";
        // Update help display for the user if they forgot the new -n flag
        std::cerr << "Required: -p (in path), -f (in name), -r (rate), -o (out path), -n (out name)\n";
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char** argv)
{
    std::string cil_matrix_path;     // example "../data/cil_matrices/"
    std::string cil_matrix_filename; // example "R_0p2_R_0p01_RA_SPPCOM_2"
    float rate;                      // example 0.04
    size_t exp_factor;
    std::string bsparse_matrix_path;     // example "../data/h_matrices/"
    std::string bsparse_matrix_filename; // example "new.bsparse"

    parse_args(
      argc, argv, cil_matrix_path, cil_matrix_filename, rate, exp_factor, bsparse_matrix_path, bsparse_matrix_filename);

    // Load matrix from cil
    CVQKD::tools::temporary_code H = CVQKD::tools::new_from_cil(cil_matrix_path, cil_matrix_filename);

    // Change rate
    CVQKD::tools::augment_rate(H, rate);

    // QC expansion
    if (exp_factor > 1)
    {
        CVQKD::tools::make_qc(H, exp_factor);
    }

    // Write matrix
    CVQKD::tools::bsparse_write(H, bsparse_matrix_path, bsparse_matrix_filename);

    return 0;
}
