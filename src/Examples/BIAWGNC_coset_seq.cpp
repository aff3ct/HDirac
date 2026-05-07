#include "Module/Decoder/LDPC/SPA/Decoder_LDPC_BP_flooding_SPA_coset.hpp"
#include "Module/Decoder/LDPC/Syndrome_computer.hpp"

#include "Tools/Helper.hpp"
#include "Tools/Reporter/Noise/Reporter_noise_CVQKD.hpp"

#include "Code/Code_params_LDPC.hpp"

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

    std::string _filename_;     // name of the file to read LDPC file
    size_t n_iter;              // number of iterations of the LDPC decoder
    CodeParamsLDPC code_params; // LDPC coset code object
};
void
init_sim_params(sim_params& s);

struct modules
{
    std::unique_ptr<spu::module::Source_random<>> source;
    std::unique_ptr<aff3ct::module::Modem_BPSK<>> modem;
    std::unique_ptr<aff3ct::module::Channel_AWGN_LLR<>> channel;
    std::unique_ptr<CVQKD::module::Syndrome_computer<>> syndrome_computer;
    std::unique_ptr<CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>> decoder;
    std::unique_ptr<aff3ct::module::Monitor_BFER<>> monitor;
};
void
init_modules(const sim_params& s, const CodeParamsLDPC& c, modules& m);

using Monitor_BFER_reduction = aff3ct::tools::Monitor_reduction<aff3ct::module::Monitor_BFER<>>;

struct utils
{
    std::unique_ptr<aff3ct::tools::Sigma<>> noise;                // a sigma noise type
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
    std::cout << "#                 ~~-- BIAWGN Channel --~~                 " << std::endl;
    std::cout << "#----------------------------------------------------------" << std::endl;
    std::cout << "#" << std::endl;

    sim_params s; // all simulation parameters
    init_sim_params(s);
    CodeParamsLDPC& c = s.code_params; // LDPC coset code object

    modules m; // AFF3CT modules
    init_modules(s, c, m);

    (*m.modem)["modulate::X_N1"] = (*m.source)["generate::out_data"];

    (*m.channel)["add_noise::X_N"] = (*m.modem)["modulate::X_N2"];

    (*m.modem)["demodulate::Y_N1"] = (*m.channel)["add_noise::Y_N"];

    (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

    (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

    (*m.decoder)["decode_siho::Y_N"] = (*m.modem)["demodulate::Y_N2"];

    (*m.monitor)["check_errors::U"] = (*m.source)["generate::out_data"];
    (*m.monitor)["check_errors::V"] = (*m.decoder)["decode_siho::V_K"];

    std::vector<float> sigma(1);
    (*m.channel)["add_noise::CP"] = sigma;
    (*m.modem)["demodulate::CP"] = sigma;

    // task order
    (*m.decoder)("decode_siho") = (*m.decoder)("set_syndrome");

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

        float snr = CVQKD::tools::beta_to_snr(beta, s.code_params.get_R());

        auto reporter_noise = static_cast<CVQKD::tools::Reporter_noise_CVQKD<>*>(u.reporters[0].get());
        reporter_noise->update(beta);

        // setting sigma from SNR : here sigma**2 = 1 / SNR
        sigma[0] = 1 / sqrt(snr);

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
        m.monitor->reset();

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
    s.refresh_fcy = 500;
    s.n_threads = 1;

    s._filename_ = "../data/h_matrices_bsparse/n131072r0p152587890625_cil_RA_SPPCOM_2.bsparse";
    s.n_iter = 500;
    s.code_params = CodeParamsLDPC(s._filename_, s.n_iter, true);

    // display parameters
    std::cout << "# * Simulation parameters: " << std::endl;
    std::cout << "#    ** Frame errors   = " << s.fe << std::endl;
    std::cout << "#    ** Max. frames    = " << s.max_frames << std::endl;
    std::cout << "#    ** Noise seed     = " << s.seed << std::endl;
    std::cout << "#    ** Info. bits (k) = " << s.code_params.get_k() << std::endl;
    std::cout << "#    ** Frame size (n) = " << s.code_params.get_n() << std::endl;
    std::cout << "#    ** Synd. size (r) = " << s.code_params.get_r() << std::endl;
    std::cout << "#    ** Code rate  (R) = " << s.code_params.get_R() << std::endl;
    std::cout << "#    ** Beta max       = " << s.beta_max * 100 << "%" << std::endl;
    std::cout << "#    ** Beta min       = " << s.beta_min * 100 << "%" << std::endl;
    std::cout << "#    ** Beta step      = " << s.beta_step * 100 << "%" << std::endl;
    std::cout << "#    ** Nb. threads    = " << s.n_threads << std::endl;
    std::cout << "#" << std::endl;
}

void
init_modules(const sim_params& s, const CodeParamsLDPC& c, modules& m)
{
    std::cout << "# " << rang::fg::blue << "(II)" << rang::fg::reset << " Performing allocations..." << std::flush;

    // modules allocations
    m.source = std::unique_ptr<spu::module::Source_random<>>(new spu::module::Source_random<>(c.get_n()));
    m.modem = std::unique_ptr<aff3ct::module::Modem_BPSK<>>(new aff3ct::module::Modem_BPSK<>(c.get_n()));
    m.channel = std::unique_ptr<aff3ct::module::Channel_AWGN_LLR<>>(new aff3ct::module::Channel_AWGN_LLR<>(c.get_n()));

    m.syndrome_computer = std::unique_ptr<CVQKD::module::Syndrome_computer<>>(
      new CVQKD::module::Syndrome_computer<>(c.get_n(), c.get_r(), c.get_H()));
    std::vector<uint32_t> bit_pos(c.get_n());
    std::iota(bit_pos.begin(), bit_pos.end(), 0);
    m.decoder = std::unique_ptr<CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>>(
      new CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>(
        c.get_n(), c.get_n(), c.get_r(), c.get_n_ite(), c.get_H(), bit_pos));
    m.monitor = std::unique_ptr<aff3ct::module::Monitor_BFER<>>(new aff3ct::module::Monitor_BFER<>(c.get_n(), s.fe));
}

void
init_utils(const sim_params& s, const modules& m, utils& u)
{
    // tasks by which the sequence starts
    u.sequence = std::unique_ptr<spu::runtime::Sequence>(
      new spu::runtime::Sequence((*m.source)("generate"), s.n_threads ? s.n_threads : 1));

    // makes a graph of the sequence
    std::ofstream f("sequence.dot");
    u.sequence->export_dot(f);

    // allocate a common monitor module to reduce all the monitors
    u.monitor_red = std::unique_ptr<Monitor_BFER_reduction>(
      new Monitor_BFER_reduction(u.sequence->get_modules<aff3ct::module::Monitor_BFER<>>()));
    u.monitor_red->set_reduce_frequency(std::chrono::milliseconds(s.refresh_fcy));

    // report the noise values
    u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(
      new CVQKD::tools::Reporter_noise_CVQKD<>(s.beta_max, s.code_params.get_R(), true)));

    // report the bit/frame error rates
    u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_BFER<>(*m.monitor)));
    // report the simulation throughputs
    u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_throughput<>(*m.monitor)));

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
