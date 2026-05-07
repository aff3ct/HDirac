#include "Module/Alice_quantum_generator.hpp"
#include "Module/LLRs_computer.hpp"
#include "Module/LLRs_formatter.hpp"
#include "Module/Norms2_computer.hpp"
#include "Module/Quantum_channel.hpp"
#include "Module/Rotation_matrices_computer.hpp"

#include "Tools/Helper.hpp"
#include "Tools/Reporter/Noise/Reporter_noise_CVQKD.hpp"

#include "Code/Code_params_LDPC_synd_cnct.hpp"

#define SHOW_STATS

struct sim_params
{
    const float ALICE_VAR = 1.0f; // Alice's variance (1 because SNR is computed as 1 / sigma^2)
    int seed;                     // PRNG seed for the AWGN channel and Alice's generator
    float beta_max;               // maximum Beta value
    float beta_min;               // minimum Beta value
    float beta_step;              // Beta step
    size_t fe;                    // max number of failed frames
    size_t max_frames;            // max number of frames
    size_t refresh_fcy;           // refresh frequency of the terminal
    size_t n_threads;             // number of threads in the sequence
    size_t d;                     // dimension of the reconciliation
    size_t sub_vec_length;        // number of sub vectors in a frame for reconciliation

    std::string _filename_;             // name of the file to read LDPC file
    size_t n_iter;                      // number of iterations of the LDPC decoder
    CodeParamsLDPCSyndCnct code_params; // LDPC syndrome concatenation code object
};
void
init_sim_params(sim_params& s);

struct modules
{
    std::unique_ptr<spu::module::Source_random<>> source;                // Bob's key generator
    std::unique_ptr<aff3ct::module::Modem_BPSK<>> modem;                 // Bob BPSK mapper
    std::unique_ptr<aff3ct::module::Decoder_LDPC_BP_flooding<>> decoder; // LDPC decoder
    std::unique_ptr<aff3ct::module::Monitor_BFER<>> monitor;             // AFF3CT monitor

    std::unique_ptr<CVQKD::module::Alice_quantum_generator<>> QRNG;    // Alice's quantum number generator
    std::unique_ptr<CVQKD::module::Quantum_channel<>> quantum_channel; // Quantum channel simulation (AWGN)
    std::unique_ptr<CVQKD::module::Norms2_computer<>> norms2_computer; // Squared norms computer
    std::unique_ptr<CVQKD::module::Rotation_matrices_computer<>>
      rotation_matrices_computer;                                  // rotation matrices computer
    std::unique_ptr<CVQKD::module::LLRs_computer<>> LLRs_computer; // LLR computer
    std::unique_ptr<CVQKD::module::LLRs_formatter<>>
      LLRs_formatter; // Used to use a systematic classical decoder as a syndrome decoder
};
void
init_modules(const sim_params& s, const CodeParamsLDPCSyndCnct& c, modules& m);

using Monitor_BFER_reduction = aff3ct::tools::Monitor_reduction<aff3ct::module::Monitor_BFER<>>;

struct utils
{
    std::vector<std::unique_ptr<spu::tools::Reporter>> reporters; // list of reporters dispayed in the terminal
    std::unique_ptr<spu::tools::Terminal_std> terminal;           // manage the output text in the terminal

    std::unique_ptr<Monitor_BFER_reduction> monitor_red; // main monitor object that reduces all the thread monitors
    std::unique_ptr<spu::runtime::Sequence> sequence;    // a sequence to run the processing chain
};
void
init_utils(const sim_params& s, const modules& m, utils& u);

int
main(int argc, char** argv)
{
    // StreamPU will catch and manage sigint
    spu::tools::Signal_handler::init();

    std::cout << "#----------------------------------------------------------" << std::endl;
    std::cout << "#             ~~-- CV-QKD Virtual Channel --~~             " << std::endl;
    std::cout << "#----------------------------------------------------------" << std::endl;
    std::cout << "#" << std::endl;

    sim_params s; // all simulation parameters
    init_sim_params(s);
    CodeParamsLDPCSyndCnct& c = s.code_params; // LDPC coset code object

    modules m; // AFF3CT modules
    init_modules(s, c, m);

    // checking if the frame length is a multiple of the dimension
    if (c.get_n() % s.d != 0)
    {
        std::cerr << rang::style::bold << rang::fg::red << "Warning " << rang::style::reset << rang::fg::reset
                  << ": the dimension is not suitable (n mod d = " << c.get_n() % s.d << ")" << std::endl;
    }

    // getting SNR from beta
    float snr = CVQKD::tools::beta_to_snr(s.beta_max, s.code_params.get_R());

    // binding all the modules and tasks to simulate the CVQKD virtual channel
    (*m.quantum_channel)["add_noise::in_X"] = (*m.QRNG)["generate::out_X"];
    (*m.quantum_channel)["add_noise::in_snr"] = &snr;

    (*m.norms2_computer)["compute_norms2::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

    (*m.modem)["modulate::X_N1"] = (*m.source)["generate::out_data"];

    (*m.rotation_matrices_computer)["compute_rotations::in_norms2"] =
      (*m.norms2_computer)["compute_norms2::out_norms2"];
    (*m.rotation_matrices_computer)["compute_rotations::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];
    (*m.rotation_matrices_computer)["compute_rotations::in_U"] = (*m.modem)["modulate::X_N2"];

    (*m.rotation_matrices_computer)["compute_reverse_rotations::in_rotation_matrices"] =
      (*m.rotation_matrices_computer)["compute_rotations::out_rotation_matrices"];
    (*m.rotation_matrices_computer)["compute_reverse_rotations::in_X"] = (*m.QRNG)["generate::out_X"];

    (*m.LLRs_computer)["compute_LLRs::in_snr"] = &snr;
    (*m.LLRs_computer)["compute_LLRs::in_rotated_X"] =
      (*m.rotation_matrices_computer)["compute_reverse_rotations::out_rotated_X"];
    (*m.LLRs_computer)["compute_LLRs::in_norms2"] = (*m.norms2_computer)["compute_norms2::out_norms2"];

    (*m.LLRs_formatter)["format_LLRs::in_U_ref"] = (*m.source)["generate::out_data"];
    (*m.LLRs_formatter)["format_LLRs::in_LLRs"] = (*m.LLRs_computer)["compute_LLRs::out_LLRs"];

    (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_formatter)["format_LLRs::out_fmt_LLRs"];

    (*m.monitor)["check_errors::U"] = (*m.source)["generate::out_data"];
    (*m.monitor)["check_errors::V"] = (*m.decoder)["decode_siho::V_K"];

    utils u;
    init_utils(s, m, u);

    // set different seeds in the modules that uses PRNG
    std::mt19937 prng(s.seed);
    for (auto& m : u.sequence->get_modules<spu::tools::Interface_set_seed>())
        m->set_seed(prng());

    const size_t MAX_FRAMES = s.max_frames;

#ifndef SHOW_STATS
    // display the legend in the terminal
    u.terminal->legend();
#endif

    // loop over the various Betas
    for (float beta = s.beta_max; beta > s.beta_min; beta -= s.beta_step)
    {
#ifdef SHOW_STATS
        // display the legend in the terminal
        u.terminal->legend();
#endif

        snr = CVQKD::tools::beta_to_snr(beta, s.code_params.get_R());

        // provide current SNR for statistics
        auto reporter_noise = static_cast<CVQKD::tools::Reporter_noise_CVQKD<>*>(u.reporters[0].get());
        reporter_noise->update(beta);

        // display the performance (BER and FER) in real time (in a separate thread)
        u.terminal->start_temp_report();

        size_t frame_counter = 0;

        // run the simulation chain
        u.sequence->exec([&u, &frame_counter, MAX_FRAMES]() -> bool
                         { return u.monitor_red->is_done() || (++frame_counter >= MAX_FRAMES); });

        // reset sigint if previously triggered
        if (spu::tools::Signal_handler::is_sigint()) spu::tools::Signal_handler::reset_sigint();

        u.monitor_red->reduce();

        // display the performance (BER and FER) in the terminal
        u.terminal->final_report();

        // reset the monitor for the next SNR
        u.monitor_red->reset();

#ifdef SHOW_STATS
        std::cout << "#" << std::endl;
        spu::tools::Stats::show(u.sequence->get_modules_per_types(), true);
        std::cout << "#\n#" << std::endl;
#endif

        for (auto& mod : u.sequence->get_modules<spu::module::Module>(false))
            for (auto& tsk : mod->tasks)
                tsk->reset();
    }

    std::cout << "#" << std::endl;
    std::cout << "# End of the simulation" << std::endl;

    return 0;
}

// initialisation of the simulation parameters
void
init_sim_params(sim_params& s)
{
    srand(time(NULL));
    s.seed = rand();
    s.beta_max = 1.0f;
    s.beta_min = 0.85f;
    s.beta_step = 0.01f;
    s.fe = 100;
    s.max_frames = 100;
    s.d = 64;
    s.refresh_fcy = 500;
    s.n_threads = 1;

    s._filename_ = "../data/h_matrices_bsparse/n131072r0p152587890625_cil_RA_SPPCOM_2.bsparse";
    s.n_iter = 500;
    s.code_params = CodeParamsLDPCSyndCnct(s._filename_, s.n_iter, true);
    s.sub_vec_length = s.code_params.get_n_orig() / s.d;

    // display parameters
    std::cout << "# * Simulation parameters: " << std::endl;
    std::cout << "#    ** Frame errors   = " << s.fe << std::endl;
    std::cout << "#    ** Max. frames    = " << s.max_frames << std::endl;
    std::cout << "#    ** Noise seed     = " << s.seed << std::endl;
    std::cout << "#    ** Info. bits (k) = " << s.code_params.get_k_orig() << std::endl;
    std::cout << "#    ** Frame size (n) = " << s.code_params.get_n_orig() << std::endl;
    std::cout << "#    ** Synd. size (r) = " << s.code_params.get_r_orig() << std::endl;
    std::cout << "#    ** Code rate  (R) = " << s.code_params.get_R() << std::endl;
    std::cout << "#    ** Beta max       = " << s.beta_max * 100 << "%" << std::endl;
    std::cout << "#    ** Beta min       = " << s.beta_min * 100 << "%" << std::endl;
    std::cout << "#    ** Beta step      = " << s.beta_step * 100 << "%" << std::endl;
    std::cout << "#    ** Reconcil. dim. = " << s.d << std::endl;
    std::cout << "#    ** Nb. threads    = " << s.n_threads << std::endl;
    std::cout << "#" << std::endl;
}

void
init_modules(const sim_params& s, const CodeParamsLDPCSyndCnct& c, modules& m)
{
    assert(c.get_k() == c.get_n_orig());
    assert(c.get_r() == c.get_r_orig());
    assert(c.get_n() == (c.get_k() + c.get_r()));

    std::cout << "# " << rang::fg::blue << "(II)" << rang::fg::reset << " Performing allocations..." << std::flush;

    // modules allocations
    m.source = std::unique_ptr<spu::module::Source_random<>>(new spu::module::Source_random<>(c.get_k()));
    m.modem = std::unique_ptr<aff3ct::module::Modem_BPSK<>>(new aff3ct::module::Modem_BPSK<>(c.get_k()));
    m.decoder =
      std::unique_ptr<aff3ct::module::Decoder_LDPC_BP_flooding<>>(new aff3ct::module::Decoder_LDPC_BP_flooding_SPA<>(
        c.get_k(), c.get_n(), c.get_n_ite(), c.get_H(), c.get_bits_pos()));
    m.monitor = std::unique_ptr<aff3ct::module::Monitor_BFER<>>(new aff3ct::module::Monitor_BFER<>(c.get_k(), s.fe));

    aff3ct::tools::Gaussian_noise_generator_std<double> generator;

    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
      new CVQKD::module::Alice_quantum_generator<>(c.get_k(), s.ALICE_VAR, generator));
    m.quantum_channel =
      std::unique_ptr<CVQKD::module::Quantum_channel<>>(new CVQKD::module::Quantum_channel<>(c.get_k(), generator));
    m.norms2_computer = std::unique_ptr<CVQKD::module::Norms2_computer<>>(
      new CVQKD::module::Norms2_computer<>(c.get_k(), s.d, s.sub_vec_length));
    m.rotation_matrices_computer = std::unique_ptr<CVQKD::module::Rotation_matrices_computer<>>(
      new CVQKD::module::Rotation_matrices_computer<>(c.get_k(), s.d, s.sub_vec_length, generator));
    m.LLRs_computer = std::unique_ptr<CVQKD::module::LLRs_computer<>>(
      new CVQKD::module::LLRs_computer<>(c.get_k(), s.d, s.sub_vec_length));
    m.LLRs_formatter = std::unique_ptr<CVQKD::module::LLRs_formatter<>>(
      new CVQKD::module::LLRs_formatter<>(c.get_k(), c.get_r(), c.get_P()));
}

void
init_utils(const sim_params& s, const modules& m, utils& u)
{
    // tasks by which the sequence starts
    std::vector<spu::runtime::Task*> first_tasks({ &(*m.QRNG)("generate"), &(*m.source)("generate") });
    u.sequence =
      std::unique_ptr<spu::runtime::Sequence>(new spu::runtime::Sequence(first_tasks, s.n_threads ? s.n_threads : 1));

    // makes a graph of the sequence
    std::ofstream f("sequence.dot");
    u.sequence->export_dot(f);

    // allocate a common monitor module to reduce all the monitors
    u.monitor_red = std::unique_ptr<Monitor_BFER_reduction>(
      new Monitor_BFER_reduction(u.sequence->get_modules<aff3ct::module::Monitor_BFER<>>()));
    u.monitor_red->set_reduce_frequency(std::chrono::milliseconds(s.refresh_fcy));

    // report the noise values (Es/N0 and Eb/N0)
    u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(
      new CVQKD::tools::Reporter_noise_CVQKD<>(s.beta_max, s.code_params.get_R())));

    // report the bit/frame error rates
    u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_BFER<>(*u.monitor_red)));
    // report the simulation throughputs
    u.reporters.push_back(
      std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_throughput<>(*u.monitor_red)));

    // create a terminal that will display the collected data from the reporters
    u.terminal = std::unique_ptr<spu::tools::Terminal_std>(new spu::tools::Terminal_std(u.reporters));

    // configuration of the sequence tasks
    for (auto& mod : u.sequence->get_modules<spu::module::Module>(false))
        for (auto& tsk : mod->tasks)
        {
            tsk->set_debug(false);    // disable the debug mode
            tsk->set_debug_limit(16); // display only the 16 first bits if the debug mode is enabled
#ifdef SHOW_STATS
            tsk->set_stats(true); // enable the statistics
#else
            tsk->set_stats(false); // enable the statistics
#endif
            // enable the fast mode (= disable the useless verifs in the tasks) if there is no debug and stats modes
            if (!tsk->is_debug() && !tsk->is_stats()) tsk->set_fast(true);
        }

    std::cout << " Done." << std::endl;
}
