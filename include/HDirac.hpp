#ifndef HDIRAC_
#define HDIRAC_

#include "Code/Code_params_LDPC.hpp"

#include "Code/Code_params_LDPC_synd_cnct.hpp"
#include "Module/Alice_quantum_generator.hpp"
#include "Module/Complex_operator.hpp"
#include "Module/Decoder/LDPC/SPA/Decoder_LDPC_BP_flooding_SPA_coset.hpp"
#include "Module/Decoder/LDPC/Syndrome_computer.hpp"
#include "Module/LLRs_computer.hpp"
#include "Module/LLRs_computer_low_dimensions.hpp"
#include "Module/LLRs_formatter.hpp"
#include "Module/Norms2_computer.hpp"
#include "Module/Octonion_operator.hpp"
#include "Module/Quantum_channel.hpp"
#include "Module/Quaternion_operator.hpp"
#include "Module/Real_operator.hpp"
#include "Module/Rotation_matrices_computer.hpp"
#include "Module/Rotation_vectors_computer.hpp"

// channel type and decoder type and AFF3CT chain type
enum class AffectSimType
{
    SEQUENCE,
    PIPELINE
};

enum class DecoderType
{
    COSET,
    COSET_SYND_CNCT
};

enum class ChannelType
{
    BIAWGN,
    REAL,
    COMPLEX,
    QUATERNION,
    OCTONION,
    HOUSEHOLDER_Od2,
    HOUSEHOLDER_Od3,
    CROSS_ROTATIONS
};

struct SimType
{
    AffectSimType affectSimType;
    DecoderType decoderType;
    ChannelType channelType;
};

struct sim_params
{
    SimType sim_type;
    const float ALICE_VAR = 10.0f; // Alice's variance (1.0f for a BIAWGNC, because the SNR is computed as 1 / sigma^2)
    int seed;                      // PRNG seed for the AWGN channel and Alice's generator
    float beta_max;                // maximum Beta value
    float beta_min;                // minimum Beta value
    float beta_step;               // Beta step
    size_t fe;                     // max number of failed frames
    size_t d;                      // dimension for virtual channel
    size_t max_frames;             // max number of frames
    size_t refresh_fcy;            // refresh frequency of the terminal
    size_t n_threads;              // number of threads in the sequence

    std::string _filename_;                      // name of the file to read LDPC file
    size_t n_iter;                               // number of iterations of the LDPC decoder
    std::unique_ptr<CodeParamsLDPC> code_params; // LDPC code object
    size_t sub_vec_length;
};
void
init_sim_params(sim_params& s);

struct modules
{
    // All possible modules (a good part of them never allocated)
    std::unique_ptr<spu::module::Source_random<>>
      source; // source in a BIAWGNC, Bob's key generator for virtual channel
    std::unique_ptr<aff3ct::module::Modem_BPSK<>> modem;
    std::unique_ptr<aff3ct::module::Channel_AWGN_LLR<>> channel;
    std::unique_ptr<CVQKD::module::Syndrome_computer<>> syndrome_computer;
    std::unique_ptr<CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>> decoder;
    std::unique_ptr<aff3ct::module::Monitor_BFER<>> monitor;

    std::unique_ptr<CVQKD::module::LLRs_formatter<>> LLRs_formatter;
    std::unique_ptr<spu::module::Initializer<int32_t>> syndrome_initializer;

    std::unique_ptr<CVQKD::module::Alice_quantum_generator<>> QRNG;    // Alice's quantum number generator
    std::unique_ptr<CVQKD::module::Quantum_channel<>> quantum_channel; // Quantum channel simulation (AWGN)

    std::unique_ptr<CVQKD::module::Real_operator<>> real_operator;             // real multiplier and divider
    std::unique_ptr<CVQKD::module::Complex_operator<>> complex_operator;       // complex multiplier and divider
    std::unique_ptr<CVQKD::module::Quaternion_operator<>> quaternion_operator; // quaternion multiplier and divider
    std::unique_ptr<CVQKD::module::Octonion_operator<>> octonion_operator;     // octonion multiplier and divider

    std::unique_ptr<CVQKD::module::LLRs_computer_low_dimensions<>>
      LLRs_computer_low_dimensions; // LLR computer for low reconciliation dimensions -Cayley-Dickson algebra- (d <= 8)

    std::unique_ptr<CVQKD::module::Rotation_matrices_computer<>>
      rotation_matrices_computer;                                                          // rotation matrices computer
    std::unique_ptr<CVQKD::module::Rotation_vectors_computer<>> rotation_vectors_computer; // rotation vectors computer

    std::unique_ptr<CVQKD::module::Norms2_computer<>> norms2_computer; // Squared norms computer
    std::unique_ptr<CVQKD::module::LLRs_computer<>> LLRs_computer;     // LLR computer for high dimensions
};
void
init_modules(const sim_params& s, modules& m);

using Monitor_BFER_reduction = aff3ct::tools::Monitor_reduction<aff3ct::module::Monitor_BFER<>>;

struct utils
{
    std::unique_ptr<aff3ct::tools::Sigma<>> noise;                // a sigma noise type
    std::vector<std::unique_ptr<spu::tools::Reporter>> reporters; // list of reporters dispayed in the terminal
    std::unique_ptr<spu::tools::Terminal_std> terminal;           // manage the output text in the terminal

    std::unique_ptr<Monitor_BFER_reduction> monitor_red; // main monitor object that reduces all the thread monitors
    std::unique_ptr<spu::runtime::Sequence> sequence;    // a sequence to run the processing chain

    std::unique_ptr<spu::runtime::Pipeline> pipeline; // a pipeline to run the processing chain
};
void
init_utils(const sim_params& s, const modules& m, utils& u, const bool stats_enabeled);

void
print_sim_banner(const sim_params& s);

void
run_sim(sim_params& s, const bool stats_enabeled = false);

#endif
