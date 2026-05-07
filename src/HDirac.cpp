#include "HDirac.hpp"
#include "Tools/Helper.hpp"
#include "Tools/Reporter/Noise/Reporter_noise_CVQKD.hpp"

inline ChannelType
parse_channel(const std::string& s)
{
    static const std::unordered_map<std::string, ChannelType> map = {
        { "biawgn", ChannelType::BIAWGN },
        { "real", ChannelType::REAL },
        { "complex", ChannelType::COMPLEX },
        { "quaternion", ChannelType::QUATERNION },
        { "octonion", ChannelType::OCTONION },
        { "householder_od2", ChannelType::HOUSEHOLDER_Od2 },
        { "householder_od3", ChannelType::HOUSEHOLDER_Od3 },

        // aliases
        { "quat", ChannelType::QUATERNION },
        { "oct", ChannelType::OCTONION },
        { "hod2", ChannelType::HOUSEHOLDER_Od2 },
        { "hod3", ChannelType::HOUSEHOLDER_Od3 }
    };

    auto it = map.find(s);
    if (it == map.end()) throw std::invalid_argument("Unknown channel type: " + s);

    return it->second;
}

inline DecoderType
parse_decoder(const std::string& s)
{
    static const std::unordered_map<std::string, DecoderType> map = { { "coset", DecoderType::COSET },
                                                                      { "coset_synd", DecoderType::COSET_SYND_CNCT },

                                                                      // alias (explicit but shorter)
                                                                      { "cs", DecoderType::COSET_SYND_CNCT } };

    auto it = map.find(s);
    if (it == map.end()) throw std::invalid_argument("Unknown decoder type: " + s);

    return it->second;
}

inline AffectSimType
parse_mode(const std::string& s)
{
    if (s == "sequence") return AffectSimType::SEQUENCE;
    if (s == "pipeline") return AffectSimType::PIPELINE;

    throw std::invalid_argument("Unknown mode: " + s);
}

inline void
validate_params(const sim_params& s)
{
    if (s._filename_.empty()) throw std::runtime_error("Missing required --file");

    if ((s.sim_type.channelType == ChannelType::HOUSEHOLDER_Od2 ||
         s.sim_type.channelType == ChannelType::HOUSEHOLDER_Od3) &&
        s.d <= 0)
    {
        throw std::runtime_error("Parameter --d must be > 0 for high-dimensional channels");
    }

    if (s.beta_min > s.beta_max) throw std::runtime_error("beta_min cannot be greater than beta_max");

    if (s.beta_min < 0.0f) throw std::runtime_error("beta_min cannot be smaller than 0");

    if (s.beta_max > 1.0f) throw std::runtime_error("beta_min cannot be higher than 1");

    if (s.beta_step <= 0.0f) throw std::runtime_error("beta_step must be > 0");

    // Applies in the current states of the simulation

    if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
    {
        if (s.sim_type.decoderType == DecoderType::COSET &&
            (s.sim_type.channelType != ChannelType::BIAWGN && s.sim_type.channelType != ChannelType::HOUSEHOLDER_Od2))
        {
            std::string channel_list = "\n\t\t-The BIAWGNC\n\t\t-The Householder O(d^2) virtual channel";
            std::string error_msg = "The channels available for a PIPELINED COSET coding scheme are: " + channel_list;
            throw std::runtime_error(error_msg);
        }
        else if (s.sim_type.decoderType == DecoderType::COSET_SYND_CNCT &&
                 (s.sim_type.channelType != ChannelType::BIAWGN))
        {
            std::string error_msg = "The BIAWGNC is the only channel available for a PIPELINED COSET coding scheme "
                                    "with SYNDROME CONCATENATION.";
            throw std::runtime_error(error_msg);
        }
    }
    else if (s.sim_type.affectSimType == AffectSimType::SEQUENCE)
    {
        if (s.sim_type.decoderType == DecoderType::COSET_SYND_CNCT && (s.sim_type.channelType != ChannelType::BIAWGN))
        {
            std::string error_msg = "The BIAWGNC is the only channel available for a SEQUENCED COSET coding scheme "
                                    "with SYNDROME CONCATENATION";
            throw std::runtime_error(error_msg);
        }
    }
}

// Help display
inline void
print_help()
{
    std::cout << rang::style::bold << "Usage:" << rang::style::reset << "./bin/HDirac [options]\n\n"
              << rang::style::bold << "Required:" << rang::style::reset
              << "\n"
                 "  --file <path>\n\n"

              << rang::style::bold << "Simulation:" << rang::style::reset
              << "\n"
                 "  --channel <type>\n"
                 "      biawgn | real | complex | quaternion | octonion\n"
                 "      householder_od2 | householder_od3\n\n"

                 "  --decoder <type>\n"
                 "      coset | coset_synd\n\n"

                 "  --mode <sequence|pipeline>\n\n"

              << rang::style::bold << "Parameters:" << rang::style::reset
              << "\n"
                 "  --d <int>\n\n"
                 "  --beta-min <float>\n"
                 "  --beta-max <float>\n"
                 "  --beta-step <float>\n\n"
                 "  --max-error-frame <float>\n"
                 "  --max-frame <int>\n\n"

              << rang::style::bold << "Other:" << rang::style::reset
              << "\n"
                 "  --stats\n"
                 "  --help\n\n"

              << rang::style::bold << "Aliases:" << rang::style::reset
              << "\n"
                 "  quat=quaternion, oct=octonion\n"
                 "  hod2=householder_od2, hod3=householder_od3\n"
                 "  cs=coset_synd\n";
}

// Main (init parameters and run)
int
main(int argc, char const* argv[])
{
    sim_params s{};
    bool stats_enabled = false;

    // Defaults parameters
    s.sim_type.channelType = ChannelType::HOUSEHOLDER_Od2;
    s.sim_type.decoderType = DecoderType::COSET;
    s.sim_type.affectSimType = AffectSimType::PIPELINE;

    s.beta_min = 0.85f;
    s.beta_max = 1.0f;
    s.beta_step = 0.01f;

    s.fe = 100;
    s.max_frames = 1000;

    s.d = 64;

    try
    {
        for (int i = 1; i < argc; i++)
        {
            std::string arg = argv[i];

            if (arg == "--help" || arg == "--h" || arg == "-h")
            {
                print_help();
                return 0;
            }
            else if (arg == "--file" && i + 1 < argc)
            {
                s._filename_ = argv[++i];
            }
            else if (arg == "--channel" && i + 1 < argc)
            {
                s.sim_type.channelType = parse_channel(argv[++i]);
            }
            else if (arg == "--decoder" && i + 1 < argc)
            {
                s.sim_type.decoderType = parse_decoder(argv[++i]);
            }
            else if (arg == "--mode" && i + 1 < argc)
            {
                s.sim_type.affectSimType = parse_mode(argv[++i]);
            }
            else if (arg == "--d" && i + 1 < argc)
            {
                s.d = std::stoi(argv[++i]);
            }
            else if (arg == "--beta-min" && i + 1 < argc)
            {
                s.beta_min = std::stof(argv[++i]);
            }
            else if (arg == "--beta-max" && i + 1 < argc)
            {
                s.beta_max = std::stof(argv[++i]);
            }
            else if (arg == "--beta-step" && i + 1 < argc)
            {
                s.beta_step = std::stof(argv[++i]);
            }
            else if (arg == "--max-frame" && i + 1 < argc)
            {
                s.max_frames = std::stof(argv[++i]);
            }
            else if (arg == "--max-error-frame" && i + 1 < argc)
            {
                s.fe = std::stof(argv[++i]);
            }
            else if (arg == "--stats")
            {
                stats_enabled = true;
            }
            else
            {
                throw std::invalid_argument("Unknown or incomplete argument: " + arg);
            }

            s.fe = std::min(s.fe, s.max_frames);
        }

        // Parameter validation
        validate_params(s);

        init_sim_params(s);
        run_sim(s, stats_enabled);
    }
    catch (const std::exception& e)
    {
        std::cerr << rang::style::bold << rang::fg::red << "\nError: " << e.what() << rang::style::reset
                  << rang::fg::reset << "\n\n";
        print_help();
        return 1;
    }

    return 0;
}

// Initialisation of the simulation parameters
void
init_sim_params(sim_params& s)
{
    srand(time(NULL));
    s.seed = rand();
    s.refresh_fcy = 500; // Terminal refresh frequency

    // Check if we are in a sequence case
    if (s.sim_type.affectSimType == AffectSimType::SEQUENCE)
    {
        s.n_threads = 1; // Should be 1
    }
    else
    {
        s.n_threads = 8;
    }

    s.n_iter = 500;
    if (s.sim_type.decoderType == DecoderType::COSET)
    {
        s.code_params.reset(new CodeParamsLDPC(s._filename_, s.n_iter, true));
    }
    else
    {
        s.code_params.reset(new CodeParamsLDPCSyndCnct(s._filename_, s.n_iter, true));
    }

    switch (s.sim_type.channelType)
    {
        case ChannelType::BIAWGN:
        {
            s.d = 0;
            s.sub_vec_length = 1;
            break;
        }

        case ChannelType::REAL:
        {
            s.d = 1;
            s.sub_vec_length = s.code_params->get_n();
            break;
        }

        case ChannelType::COMPLEX:
        {
            s.d = 2;
            s.sub_vec_length = s.code_params->get_n() / s.d;
            break;
        }

        case ChannelType::QUATERNION:
        {
            s.d = 4;
            s.sub_vec_length = s.code_params->get_n() / s.d;
            break;
        }

        case ChannelType::OCTONION:
        {
            s.d = 8;
            s.sub_vec_length = s.code_params->get_n() / s.d;
            break;
        }

        case ChannelType::HOUSEHOLDER_Od3:
        case ChannelType::HOUSEHOLDER_Od2:
        {
            s.sub_vec_length = s.code_params->get_n() / s.d;
            break;
        }

        default:
        {
            break;
        }
    }

    // Display parameters
    std::cout << "# * Simulation parameters: " << std::endl;
    std::cout << "#    ** Max. frame errors = " << s.fe << std::endl;
    std::cout << "#    ** Max. frame total  = " << s.max_frames << std::endl;
    std::cout << "#    ** Noise seed        = " << s.seed << std::endl;
    std::cout << "#    ** Info. bits (k)    = " << s.code_params->get_k() << std::endl;
    std::cout << "#    ** Frame size (n)    = " << s.code_params->get_n() << std::endl;
    std::cout << "#    ** Synd. size (r)    = " << s.code_params->get_r() << std::endl;
    std::cout << "#    ** Code rate  (R)    = " << s.code_params->get_R() << std::endl;
    std::cout << "#    ** Beta max          = " << s.beta_max * 100 << "%" << std::endl;
    std::cout << "#    ** Beta min          = " << s.beta_min * 100 << "%" << std::endl;
    std::cout << "#    ** Beta step         = " << s.beta_step * 100 << "%" << std::endl;
    if (s.sim_type.channelType != ChannelType::BIAWGN)
    {
        std::cout << "#    ** Reconcil. dim.    = " << s.d << std::endl;
    }
    if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
    {
        std::cout << "#    ** Nb. threads       = " << s.n_threads << std::endl;
    }
    std::cout << "#" << std::endl;
}

void
init_modules(const sim_params& s, modules& m)
{
    aff3ct::tools::Gaussian_noise_generator_std<double> generator;

    // Modules allocations
    switch (s.sim_type.decoderType)
    {
        case DecoderType::COSET:
        {
            auto& c = *s.code_params;

            m.source = std::unique_ptr<spu::module::Source_random<>>(new spu::module::Source_random<>(c.get_n()));
            m.modem = std::unique_ptr<aff3ct::module::Modem_BPSK<>>(new aff3ct::module::Modem_BPSK<>(c.get_n()));
            m.syndrome_computer = std::unique_ptr<CVQKD::module::Syndrome_computer<>>(
              new CVQKD::module::Syndrome_computer<>(c.get_n(), c.get_r(), c.get_H()));

            switch (s.sim_type.channelType)
            {
                case ChannelType::BIAWGN:
                {
                    m.channel = std::unique_ptr<aff3ct::module::Channel_AWGN_LLR<>>(
                      new aff3ct::module::Channel_AWGN_LLR<>(c.get_n()));
                    break;
                }

                case ChannelType::REAL:
                {
                    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
                      new CVQKD::module::Alice_quantum_generator<>(c.get_n(), s.ALICE_VAR, generator));
                    m.quantum_channel = std::unique_ptr<CVQKD::module::Quantum_channel<>>(
                      new CVQKD::module::Quantum_channel<>(c.get_n(), generator, s.ALICE_VAR));

                    m.real_operator = std::unique_ptr<CVQKD::module::Real_operator<>>(
                      new CVQKD::module::Real_operator<>(c.get_n(), s.d, s.sub_vec_length));
                    m.LLRs_computer_low_dimensions = std::unique_ptr<CVQKD::module::LLRs_computer_low_dimensions<>>(
                      new CVQKD::module::LLRs_computer_low_dimensions<>(c.get_n(), s.d, s.sub_vec_length, s.ALICE_VAR));
                    break;
                }

                case ChannelType::COMPLEX:
                {
                    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
                      new CVQKD::module::Alice_quantum_generator<>(c.get_n(), s.ALICE_VAR, generator));
                    m.quantum_channel = std::unique_ptr<CVQKD::module::Quantum_channel<>>(
                      new CVQKD::module::Quantum_channel<>(c.get_n(), generator, s.ALICE_VAR));
                    m.complex_operator = std::unique_ptr<CVQKD::module::Complex_operator<>>(
                      new CVQKD::module::Complex_operator<>(c.get_n(), s.d, s.sub_vec_length));
                    m.LLRs_computer_low_dimensions = std::unique_ptr<CVQKD::module::LLRs_computer_low_dimensions<>>(
                      new CVQKD::module::LLRs_computer_low_dimensions<>(c.get_n(), s.d, s.sub_vec_length, s.ALICE_VAR));
                    break;
                }

                case ChannelType::QUATERNION:
                {
                    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
                      new CVQKD::module::Alice_quantum_generator<>(c.get_n(), s.ALICE_VAR, generator));
                    m.quantum_channel = std::unique_ptr<CVQKD::module::Quantum_channel<>>(
                      new CVQKD::module::Quantum_channel<>(c.get_n(), generator, s.ALICE_VAR));
                    m.quaternion_operator = std::unique_ptr<CVQKD::module::Quaternion_operator<>>(
                      new CVQKD::module::Quaternion_operator<>(c.get_n(), s.d, s.sub_vec_length));
                    m.LLRs_computer_low_dimensions = std::unique_ptr<CVQKD::module::LLRs_computer_low_dimensions<>>(
                      new CVQKD::module::LLRs_computer_low_dimensions<>(c.get_n(), s.d, s.sub_vec_length, s.ALICE_VAR));
                    break;
                }

                case ChannelType::OCTONION:
                {
                    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
                      new CVQKD::module::Alice_quantum_generator<>(c.get_n(), s.ALICE_VAR, generator));
                    m.quantum_channel = std::unique_ptr<CVQKD::module::Quantum_channel<>>(
                      new CVQKD::module::Quantum_channel<>(c.get_n(), generator, s.ALICE_VAR));
                    m.octonion_operator = std::unique_ptr<CVQKD::module::Octonion_operator<>>(
                      new CVQKD::module::Octonion_operator<>(c.get_n(), s.d, s.sub_vec_length));
                    m.LLRs_computer_low_dimensions = std::unique_ptr<CVQKD::module::LLRs_computer_low_dimensions<>>(
                      new CVQKD::module::LLRs_computer_low_dimensions<>(c.get_n(), s.d, s.sub_vec_length, s.ALICE_VAR));
                    break;
                }

                case ChannelType::HOUSEHOLDER_Od3:
                {
                    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
                      new CVQKD::module::Alice_quantum_generator<>(c.get_n(), s.ALICE_VAR, generator));
                    m.quantum_channel = std::unique_ptr<CVQKD::module::Quantum_channel<>>(
                      new CVQKD::module::Quantum_channel<>(c.get_n(), generator, s.ALICE_VAR));

                    m.norms2_computer = std::unique_ptr<CVQKD::module::Norms2_computer<>>(
                      new CVQKD::module::Norms2_computer<>(c.get_n(), s.d, s.sub_vec_length));
                    m.rotation_matrices_computer = std::unique_ptr<CVQKD::module::Rotation_matrices_computer<>>(
                      new CVQKD::module::Rotation_matrices_computer<>(c.get_n(), s.d, s.sub_vec_length, generator));
                    m.LLRs_computer = std::unique_ptr<CVQKD::module::LLRs_computer<>>(
                      new CVQKD::module::LLRs_computer<>(c.get_n(), s.d, s.sub_vec_length, s.ALICE_VAR));
                    break;
                }

                case ChannelType::HOUSEHOLDER_Od2:
                {
                    m.QRNG = std::unique_ptr<CVQKD::module::Alice_quantum_generator<>>(
                      new CVQKD::module::Alice_quantum_generator<>(c.get_n(), s.ALICE_VAR, generator));
                    m.quantum_channel = std::unique_ptr<CVQKD::module::Quantum_channel<>>(
                      new CVQKD::module::Quantum_channel<>(c.get_n(), generator, s.ALICE_VAR));

                    m.norms2_computer = std::unique_ptr<CVQKD::module::Norms2_computer<>>(
                      new CVQKD::module::Norms2_computer<>(c.get_n(), s.d, s.sub_vec_length));
                    m.rotation_vectors_computer = std::unique_ptr<CVQKD::module::Rotation_vectors_computer<>>(
                      new CVQKD::module::Rotation_vectors_computer<>(c.get_n(), s.d, s.sub_vec_length, generator));
                    m.LLRs_computer = std::unique_ptr<CVQKD::module::LLRs_computer<>>(
                      new CVQKD::module::LLRs_computer<>(c.get_n(), s.d, s.sub_vec_length, s.ALICE_VAR));
                    break;
                }

                default:
                {
                    break;
                }
            }

            std::vector<uint32_t> bit_pos(c.get_n()); // Fake bit positions
            std::iota(bit_pos.begin(), bit_pos.end(), 0);
            m.decoder = std::unique_ptr<CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>>(
              new CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>(
                c.get_n(), c.get_n(), c.get_r(), c.get_n_ite(), c.get_H(), bit_pos));

            m.monitor =
              std::unique_ptr<aff3ct::module::Monitor_BFER<>>(new aff3ct::module::Monitor_BFER<>(c.get_n(), s.fe));
            break;
        }

        case DecoderType::COSET_SYND_CNCT:
        {
            auto& c = dynamic_cast<CodeParamsLDPCSyndCnct&>(*s.code_params);

            assert(c.get_k() == c.get_n_orig());
            assert(c.get_r() == c.get_r_orig());
            assert(c.get_n() == (c.get_k() + c.get_r()));

            switch (s.sim_type.channelType)
            {
                case ChannelType::BIAWGN:
                {
                    m.source =
                      std::unique_ptr<spu::module::Source_random<>>(new spu::module::Source_random<>(c.get_k()));
                    m.modem =
                      std::unique_ptr<aff3ct::module::Modem_BPSK<>>(new aff3ct::module::Modem_BPSK<>(c.get_k()));
                    m.channel = std::unique_ptr<aff3ct::module::Channel_AWGN_LLR<>>(
                      new aff3ct::module::Channel_AWGN_LLR<>(c.get_k()));

                    m.decoder = std::unique_ptr<CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>>(
                      new CVQKD::module::Decoder_LDPC_BP_flooding_SPA_coset<>(
                        c.get_k(), c.get_n(), c.get_r(), c.get_n_ite(), c.get_H(), c.get_bits_pos()));

                    m.LLRs_formatter = std::unique_ptr<CVQKD::module::LLRs_formatter<>>(
                      new CVQKD::module::LLRs_formatter<>(c.get_k(), c.get_r(), c.get_P()));

                    m.syndrome_initializer = std::unique_ptr<spu::module::Initializer<int32_t>>(
                      new spu::module::Initializer<int32_t>(c.get_r()));

                    m.syndrome_initializer->set_init_data(0);
                    break;
                }

                default:
                {
                    break;
                }
            }

            m.monitor =
              std::unique_ptr<aff3ct::module::Monitor_BFER<>>(new aff3ct::module::Monitor_BFER<>(c.get_k(), s.fe));
            break;
        }

        default:
        {
            break;
        }
    }
}

void
init_utils(const sim_params& s, const modules& m, utils& u, const bool stats_enabeled)
{
    if (s.sim_type.affectSimType == AffectSimType::SEQUENCE)
    {
        std::ofstream f("sequence.dot");

        // Tasks by which the sequence starts
        if (s.sim_type.channelType == ChannelType::BIAWGN)
        {
            switch (s.sim_type.decoderType)
            {
                case DecoderType::COSET:
                {
                    u.sequence = std::unique_ptr<spu::runtime::Sequence>(
                      new spu::runtime::Sequence((*m.source)("generate"), s.n_threads ? s.n_threads : 1));
                    break;
                }

                case DecoderType::COSET_SYND_CNCT:
                {
                    std::vector<runtime::Task*> first_tasks(
                      { &(*m.source)("generate"), &(*m.syndrome_initializer)("initialize") });
                    u.sequence = std::unique_ptr<spu::runtime::Sequence>(
                      new spu::runtime::Sequence(first_tasks, s.n_threads ? s.n_threads : 1));
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
        else
        {
            std::vector<spu::runtime::Task*> first_tasks({ &(*m.QRNG)("generate"), &(*m.source)("generate") });
            u.sequence = std::unique_ptr<spu::runtime::Sequence>(
              new spu::runtime::Sequence(first_tasks, s.n_threads ? s.n_threads : 1));
        }

        // Makes a graph of the sequence
        u.sequence->export_dot(f);

        // Allocate a common monitor module to reduce all the monitors
        u.monitor_red = std::unique_ptr<Monitor_BFER_reduction>(
          new Monitor_BFER_reduction(u.sequence->get_modules<aff3ct::module::Monitor_BFER<>>()));
        u.monitor_red->set_reduce_frequency(std::chrono::milliseconds(s.refresh_fcy));

        // Report the noise values
        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new CVQKD::tools::Reporter_noise_CVQKD<>(
          s.beta_max, s.code_params->get_R(), s.sim_type.channelType == ChannelType::BIAWGN)));

        // Report the bit/frame error rates
        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_BFER<>(*m.monitor)));
        // Report the simulation throughputs
        u.reporters.push_back(
          std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_throughput<>(*m.monitor)));

        // Create a terminal that will display the collected data from the reporters
        u.terminal = std::unique_ptr<spu::tools::Terminal_std>(new spu::tools::Terminal_std(u.reporters));

        // Configuration of the sequence tasks
        for (auto& mod : u.sequence->get_modules<spu::module::Module>(false))
        {
            for (auto& tsk : mod->tasks)
            {
                tsk->set_debug(false);          // Disable the debug mode
                tsk->set_debug_limit(16);       // Display only the 16 first bits if the debug mode is enabled
                tsk->set_stats(stats_enabeled); // Enable the statistics

                // Enable the fast mode (= disable the useless verifs in the tasks) if there is no debug and
                // Stats modes
                if (!tsk->is_debug() && !tsk->is_stats()) tsk->set_fast(true);
            }
        }
    }
    else if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
    {
        switch (s.sim_type.channelType)
        {
            case ChannelType::BIAWGN:
            {
                switch (s.sim_type.decoderType)
                {
                    case DecoderType::COSET:
                    {
                        u.pipeline.reset(
                          spu::tools::Pipeline_builder()
                            .add_task_for_checking((*m.source)("generate"))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // ------------------------------------------- STAGE 0
                                           .set_first_tasks({ &(*m.source)("generate") })
                                           .set_last_tasks({ &(*m.modem)("demodulate") })
                                           .set_excluded_tasks({ &(*m.decoder)("set_syndrome"),
                                                                 &(*m.decoder)("decode_siho"),
                                                                 &(*m.monitor)("check_errors") })
                                           .set_n_threads(1))
                            // ------------------------------------------------------------------------------------------------------------
                            .configure_interstage_synchro(
                              spu::tools::Pipeline_builder::Synchro_builder() // ---------- INTER-STAGE 0 <-> 1
                                .set_buffer_size(1)
                                .set_active_waiting(false))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(
                              spu::tools::Pipeline_builder::
                                Stage_builder() // ------------------------------------------- STAGE 2
                                  .set_first_tasks({ &(*m.decoder)("set_syndrome"), &(*m.decoder)("decode_siho") })
                                  .add_last_task((*m.decoder)("decode_siho"))
                                  .set_n_threads(s.n_threads))
                            // ------------------------------------------------------------------------------------------------------------
                            .configure_interstage_synchro(
                              spu::tools::Pipeline_builder::Synchro_builder() // ---------- INTER-STAGE 2 <-> 3
                                .set_buffer_size(1)
                                .set_active_waiting(false))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // -------------------------------------------
                                                         // STAGE 3
                                           .add_first_task((*m.monitor)("check_errors"))
                                           .add_last_task((*m.monitor)("check_errors"))
                                           .set_n_threads(1))
                            // ------------------------------------------------------------------------------------------------------------
                            .build_ptr()); //               Finally allocate and initialise a new pipeline from all the
                                           //               previous parameters
                        break;
                    }
                    case DecoderType::COSET_SYND_CNCT:
                    {
                        u.pipeline.reset(
                          spu::tools::Pipeline_builder()
                            .set_tasks_for_checking(
                              { &(*m.source)("generate"), &(*m.syndrome_initializer)("initialize") })
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // ------------------------------------------- STAGE 0
                                           .set_first_tasks({ &(*m.source)("generate") })
                                           .set_last_tasks({ &(*m.LLRs_formatter)("format_LLRs") })
                                           .set_excluded_tasks({ &(*m.decoder)("set_syndrome"),
                                                                 &(*m.decoder)("decode_siho"),
                                                                 &(*m.monitor)("check_errors") })
                                           .set_n_threads(1))
                            // ------------------------------------------------------------------------------------------------------------
                            .configure_interstage_synchro(
                              spu::tools::Pipeline_builder::Synchro_builder() // ---------- INTER-STAGE 0 <-> 1
                                .set_buffer_size(1)
                                .set_active_waiting(false))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // ------------------------------------------- STAGE 2
                                           .set_first_tasks(
                                             { &(*m.syndrome_initializer)("initialize"), &(*m.decoder)("decode_siho") })
                                           .add_last_task((*m.decoder)("decode_siho"))
                                           .set_n_threads(s.n_threads))
                            // ------------------------------------------------------------------------------------------------------------
                            .configure_interstage_synchro(
                              spu::tools::Pipeline_builder::Synchro_builder() // ---------- INTER-STAGE 2 <-> 3
                                .set_buffer_size(1)
                                .set_active_waiting(false))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // -------------------------------------------
                                                         // STAGE 3
                                           .add_first_task((*m.monitor)("check_errors"))
                                           .add_last_task((*m.monitor)("check_errors"))
                                           .set_n_threads(1))
                            // ------------------------------------------------------------------------------------------------------------
                            .build_ptr()); //               Finally allocate and initialise a new pipeline from all the
                                           //               previous parameters
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
                break;
            }

            case ChannelType::HOUSEHOLDER_Od2:
            {
                switch (s.sim_type.decoderType)
                {
                    case DecoderType::COSET:
                    {
                        u.pipeline.reset(
                          spu::tools::Pipeline_builder()
                            .set_tasks_for_checking({ &(*m.QRNG)("generate"), &(*m.source)("generate") })
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // ------------------------------------------- STAGE 0
                                           .set_first_tasks({ &(*m.QRNG)("generate"), &(*m.source)("generate") })

                                           .set_last_tasks({ &(*m.LLRs_computer)("compute_LLRs"),
                                                             &(*m.syndrome_computer)("compute_syndrome") })

                                           .set_excluded_tasks({ &(*m.decoder)("set_syndrome"),
                                                                 &(*m.decoder)("decode_siho"),
                                                                 &(*m.monitor)("check_errors") })

                                           .set_n_threads(1))
                            // ------------------------------------------------------------------------------------------------------------
                            .configure_interstage_synchro(
                              spu::tools::Pipeline_builder::Synchro_builder() // ---------- INTER-STAGE 0 <-> 1
                                .set_buffer_size(1)
                                .set_active_waiting(false))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(
                              spu::tools::Pipeline_builder::
                                Stage_builder() // ------------------------------------------- STAGE 1
                                  .set_first_tasks({ &(*m.decoder)("set_syndrome"), &(*m.decoder)("decode_siho") })
                                  .add_last_task((*m.decoder)("decode_siho"))
                                  .set_n_threads(s.n_threads))
                            // ------------------------------------------------------------------------------------------------------------
                            .configure_interstage_synchro(
                              spu::tools::Pipeline_builder::Synchro_builder() // ---------- INTER-STAGE 1 <-> 2
                                .set_buffer_size(1)
                                .set_active_waiting(false))
                            // ------------------------------------------------------------------------------------------------------------
                            .add_stage(spu::tools::Pipeline_builder::
                                         Stage_builder() // -------------------------------------------
                                                         // STAGE 2
                                           .add_first_task((*m.monitor)("check_errors"))
                                           .add_last_task((*m.monitor)("check_errors"))
                                           .set_n_threads(1))
                            // ------------------------------------------------------------------------------------------------------------
                            .build_ptr()); //               Finally allocate and initialise a new pipeline from all the
                                           //               previous parameters
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
                break;
            }

            default:
                break;
        }

        // Makes a graph of the pipline
        std::ofstream f("debug_pipeline.dot");
        u.pipeline->export_dot(f);

        // Report the noise values
        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new CVQKD::tools::Reporter_noise_CVQKD<>(
          s.beta_max, s.code_params->get_R(), s.sim_type.channelType == ChannelType::BIAWGN)));

        // Report the bit/frame error rates
        u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_BFER<>(*m.monitor)));
        // Report the simulation throughputs
        u.reporters.push_back(
          std::unique_ptr<spu::tools::Reporter>(new aff3ct::tools::Reporter_throughput<>(*m.monitor)));

        // Create a terminal that will display the collected data from the reporters
        u.terminal = std::unique_ptr<spu::tools::Terminal_std>(new spu::tools::Terminal_std(u.reporters));

        // Configuration of the pipeline tasks
        for (auto& mod : u.pipeline->get_modules<spu::module::Module>(false))
        {
            for (auto& tsk : mod->tasks)
            {
                tsk->set_debug(false);          // Disable the debug mode
                tsk->set_debug_limit(16);       // Display only the 16 first bits if the debug mode is enabled
                tsk->set_stats(stats_enabeled); // Enable the statistics
                // Enable the fast mode (= disable the useless verifs in the tasks) if there is no debug and stats modes
                if (!tsk->is_debug() && !tsk->is_stats()) tsk->set_fast(true);
            }
        }
    }
}

void
print_sim_banner(const sim_params& s)
{
    std::string channel_str;

    switch (s.sim_type.channelType)
    {
        case ChannelType::BIAWGN:
        {
            channel_str = "BIAWGN Channel";
            break;
        }
        case ChannelType::REAL:
        {
            channel_str = "CV-QKD Virtual Channel [reals]";
            break;
        }
        case ChannelType::COMPLEX:
        {
            channel_str = "CV-QKD Virtual Channel [complexes]";
            break;
        }
        case ChannelType::QUATERNION:
        {
            channel_str = "CV-QKD Virtual Channel [quaternions]";
            break;
        }
        case ChannelType::OCTONION:
        {
            channel_str = "CV-QKD Virtual Channel [octonions]";
            break;
        }
        case ChannelType::HOUSEHOLDER_Od3:
        {
            channel_str = "CV-QKD Virtual Channel [Householder O(d^3)]";
            break;
        }
        case ChannelType::HOUSEHOLDER_Od2:
        {
            channel_str = "CV-QKD Virtual Channel [Householder O(d^2)]";
            break;
        }
        default:
        {
            channel_str = "Unknown Channel";
            break;
        }
    }

    std::string mode_str =
      (s.sim_type.affectSimType == AffectSimType::PIPELINE) ? "Pipeline (parallelised decoder)" : "Sequence";

    std::string decoder_str;
    switch (s.sim_type.decoderType)
    {
        case DecoderType::COSET:
        {
            decoder_str = "Coset";
            break;
        }
        case DecoderType::COSET_SYND_CNCT:
        {
            decoder_str = "Coset with a syndrome concatenation overlay";
            break;
        }
        default:
        {
            decoder_str = "Unknown";
            break;
        }
    }

    const size_t width = 100;
    const size_t tabsize = 8;

    auto visual_length = [&](const std::string& s, size_t offset = 0)
    {
        size_t len = offset;

        for (char c : s)
        {
            if (c == '\t')
                len += tabsize - (len % tabsize);
            else
                len += 1;
        }

        return len - offset;
    };

    auto pad_right = [&](const std::string& s, size_t w)
    {
        size_t len = visual_length(s, 2);

        if (len >= w) return s;
        return s + std::string(w - len, ' ');
    };

    auto center = [&](const std::string& s, size_t w)
    {
        size_t len = visual_length(s);
        if (len >= w) return s;

        size_t space = w - len;
        size_t left = space / 2;
        size_t right = space - left;

        return std::string(left, ' ') + s + std::string(right, ' ');
    };

    auto line_left = [&](const std::string& s) { return "# " + pad_right(s, width - 3) + "#"; };

    auto line_center = [&](const std::string& s) { return "#" + center(s, width - 2) + "#"; };

    std::string sep_eq(width - 2, '=');
    std::string sep_dash(width - 2, '-');
    std::string empty(width - 2, ' ');

    std::string title = "~~-- " + channel_str + " --~~";

    std::cout << "#" << sep_eq << "#\n";
    std::cout << "#" << empty << "#\n";

    std::cout << line_center(title) << "\n";

    std::cout << "#" << empty << "#\n";
    std::cout << "#" << sep_dash << "#\n";
    std::cout << "#" << empty << "#\n";

    std::cout << line_left("\tDecoder      : " + decoder_str) << "\n";
    std::cout << line_left("\tAFF3CT chain : " + mode_str) << "\n";

    std::cout << "#" << empty << "#\n";
    std::cout << "#" << sep_eq << "#\n";
    std::cout << "#\n";
}

void
run_sim(sim_params& s, const bool stats_enabeled)
{
    // StreamPU will catch and manage sigint
    spu::tools::Signal_handler::init();

    const AffectSimType affect_sim_type = s.sim_type.affectSimType;
    const DecoderType decoder_type = s.sim_type.decoderType;
    const ChannelType channel_type = s.sim_type.channelType;

    print_sim_banner(s);

    std::cout << "# " << rang::fg::blue << "(II)" << rang::fg::reset << " Performing allocations..." << std::flush;

    modules m; // AFF3CT modules
    init_modules(s, m);

    if (s.d != 0 && s.code_params->get_n() % s.d != 0)
    {
        std::cerr << rang::style::bold << rang::fg::red << "Warning " << rang::style::reset << rang::fg::reset
                  << ": the dimension is not suitable (n mod d = " << s.code_params->get_n() % s.d << ")" << std::endl;
    }

    std::vector<float> sigma(1);
    float snr = CVQKD::tools::beta_to_snr(s.beta_max, s.code_params->get_R(), channel_type == ChannelType::BIAWGN);

    // Coset decoder is always used here (even with a syndrome concatenation overlay)
    // Because it is an optimised version of the AFF3CT decoder
    if (channel_type == ChannelType::BIAWGN)
    {
        (*m.modem)["modulate::X_N1"] = (*m.source)["generate::out_data"];

        (*m.channel)["add_noise::X_N"] = (*m.modem)["modulate::X_N2"];

        (*m.modem)["demodulate::Y_N1"] = (*m.channel)["add_noise::Y_N"];

        switch (decoder_type)
        {
            case DecoderType::COSET:
            {
                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.modem)["demodulate::Y_N2"];
                break;
            }

            case DecoderType::COSET_SYND_CNCT:
            {
                (*m.LLRs_formatter)["format_LLRs::in_U_ref"] = (*m.source)["generate::out_data"];
                (*m.LLRs_formatter)["format_LLRs::in_LLRs"] = (*m.modem)["demodulate::Y_N2"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_initializer)["initialize::out"];
                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_formatter)["format_LLRs::out_fmt_LLRs"];

                break;
            }

            default:
            {
            }
        }

        (*m.monitor)["check_errors::U"] = (*m.source)["generate::out_data"];
        (*m.monitor)["check_errors::V"] = (*m.decoder)["decode_siho::V_K"];

        (*m.channel)["add_noise::CP"] = sigma;
        (*m.modem)["demodulate::CP"] = sigma;

        // Task order in a coset case
        (*m.decoder)("decode_siho") = (*m.decoder)("set_syndrome");
    }
    else
    {
        (*m.quantum_channel)["add_noise::in_X"] = (*m.QRNG)["generate::out_X"];
        (*m.quantum_channel)["add_noise::in_snr"] = &snr;

        (*m.modem)["modulate::X_N1"] = (*m.source)["generate::out_data"];

        switch (channel_type)
        {
            case ChannelType::REAL:
            {

                (*m.real_operator)["map::in_U"] = (*m.modem)["modulate::X_N2"];
                (*m.real_operator)["map::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

                (*m.real_operator)["demap::in_mappings"] = (*m.real_operator)["map::out_mappings"];
                (*m.real_operator)["demap::in_X"] = (*m.QRNG)["generate::out_X"];

                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_snr"] = &snr;
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_X"] = (*m.QRNG)["generate::out_X"];
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_quotients"] =
                  (*m.real_operator)["demap::out_demappings"];

                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::out_LLRs"];
                break;
            }

            case ChannelType::COMPLEX:
            {

                (*m.complex_operator)["multiply::in_U"] = (*m.modem)["modulate::X_N2"];
                (*m.complex_operator)["multiply::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

                (*m.complex_operator)["divide::in_complex_products"] =
                  (*m.complex_operator)["multiply::out_complex_products"];
                (*m.complex_operator)["divide::in_X"] = (*m.QRNG)["generate::out_X"];

                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_snr"] = &snr;
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_X"] = (*m.QRNG)["generate::out_X"];
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_quotients"] =
                  (*m.complex_operator)["divide::out_complex_quotients"];

                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::out_LLRs"];
                break;
            }

            case ChannelType::QUATERNION:
            {

                (*m.quaternion_operator)["multiply::in_U"] = (*m.modem)["modulate::X_N2"];
                (*m.quaternion_operator)["multiply::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

                (*m.quaternion_operator)["divide::in_quaternion_products"] =
                  (*m.quaternion_operator)["multiply::out_quaternion_products"];
                (*m.quaternion_operator)["divide::in_X"] = (*m.QRNG)["generate::out_X"];

                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_snr"] = &snr;
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_X"] = (*m.QRNG)["generate::out_X"];
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_quotients"] =
                  (*m.quaternion_operator)["divide::out_quaternion_quotients"];

                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::out_LLRs"];
                break;
            }

            case ChannelType::OCTONION:
            {

                (*m.octonion_operator)["multiply::in_U"] = (*m.modem)["modulate::X_N2"];
                (*m.octonion_operator)["multiply::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

                (*m.octonion_operator)["divide::in_octonion_products"] =
                  (*m.octonion_operator)["multiply::out_octonion_products"];
                (*m.octonion_operator)["divide::in_X"] = (*m.QRNG)["generate::out_X"];

                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_snr"] = &snr;
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_X"] = (*m.QRNG)["generate::out_X"];
                (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::in_quotients"] =
                  (*m.octonion_operator)["divide::out_octonion_quotients"];

                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_computer_low_dimensions)["compute_LLRs_low_d::out_LLRs"];
                break;
            }

            case ChannelType::HOUSEHOLDER_Od3:
            {

                (*m.norms2_computer)["compute_norms2::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

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

                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_computer)["compute_LLRs::out_LLRs"];
                break;
            }

            case ChannelType::HOUSEHOLDER_Od2:
            {

                (*m.norms2_computer)["compute_norms2::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];

                (*m.rotation_vectors_computer)["compute_rotations::in_norms2"] =
                  (*m.norms2_computer)["compute_norms2::out_norms2"];
                (*m.rotation_vectors_computer)["compute_rotations::in_Y"] = (*m.quantum_channel)["add_noise::out_Y"];
                (*m.rotation_vectors_computer)["compute_rotations::in_U"] = (*m.modem)["modulate::X_N2"];

                (*m.rotation_vectors_computer)["compute_reverse_rotations::in_rotation_vectors"] =
                  (*m.rotation_vectors_computer)["compute_rotations::out_rotation_vectors"];
                (*m.rotation_vectors_computer)["compute_reverse_rotations::in_X"] = (*m.QRNG)["generate::out_X"];

                (*m.LLRs_computer)["compute_LLRs::in_snr"] = &snr;
                (*m.LLRs_computer)["compute_LLRs::in_rotated_X"] =
                  (*m.rotation_vectors_computer)["compute_reverse_rotations::out_rotated_X"];
                (*m.LLRs_computer)["compute_LLRs::in_norms2"] = (*m.norms2_computer)["compute_norms2::out_norms2"];

                (*m.syndrome_computer)["compute_syndrome::in_codeword"] = (*m.source)["generate::out_data"];

                (*m.decoder)["set_syndrome::in_syndrome"] = (*m.syndrome_computer)["compute_syndrome::out_syndrome"];

                (*m.decoder)["decode_siho::Y_N"] = (*m.LLRs_computer)["compute_LLRs::out_LLRs"];
                break;
            }

            default:
            {
                break;
            }
        }

        (*m.monitor)["check_errors::U"] = (*m.source)["generate::out_data"];
        (*m.monitor)["check_errors::V"] = (*m.decoder)["decode_siho::V_K"];

        // Task order in a coset case
        (*m.decoder)("decode_siho") = (*m.decoder)("set_syndrome");
    }

    utils u;
    init_utils(s, m, u, stats_enabeled);
    std::cout << " Done.\n#" << std::endl;

    // Set different seeds in the modules that uses PRNG
    if (affect_sim_type == AffectSimType::SEQUENCE)
    {
        std::mt19937 prng(s.seed);
        for (auto& m : u.sequence->get_modules<spu::tools::Interface_set_seed>())
            m->set_seed(prng());
    }
    else if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
    {
        std::mt19937 prng(s.seed);
        for (auto& m : u.pipeline->get_modules<spu::tools::Interface_set_seed>())
            m->set_seed(prng());
    }

    const size_t MAX_FRAMES = s.max_frames;

    if (!stats_enabeled)
    {
        // Display the legend in the terminal
        u.terminal->legend();
    }

    // Loop over the various Betas
    for (float beta = s.beta_max; beta > s.beta_min; beta -= s.beta_step)
    {
        if (stats_enabeled)
        {
            // Display the legend in the terminal
            u.terminal->legend();
        }

        snr = CVQKD::tools::beta_to_snr(beta, s.code_params->get_R(), channel_type == ChannelType::BIAWGN);

        auto reporter_noise = static_cast<CVQKD::tools::Reporter_noise_CVQKD<>*>(u.reporters[0].get());
        reporter_noise->update(beta);

        if (channel_type == ChannelType::BIAWGN)
        {
            // Setting sigma (noise st. dev.) from SNR : here sigma**2 = 1.0f / SNR
            // Signal variance is 1.0 since symbols are uniformly distributed over {-1, +1}
            sigma[0] = std::sqrt(1.0f / snr); // signal_variance
        }

        // Display the performance (BER and FER) in real time (in a separate thread)
        u.terminal->start_temp_report();

        size_t frame_counter = 0;

        // Run the simulation chain
        if (affect_sim_type == AffectSimType::SEQUENCE)
        {
            u.sequence->exec([&u, &frame_counter, MAX_FRAMES]() -> bool
                             { return u.monitor_red->is_done() || (++frame_counter >= MAX_FRAMES); });
        }
        else if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
        {
            u.pipeline->exec([&u, &frame_counter, MAX_FRAMES]() -> bool { return ++frame_counter >= MAX_FRAMES; });
        }

        // Reset sigint if previously triggered
        if (spu::tools::Signal_handler::is_sigint()) spu::tools::Signal_handler::reset_sigint();

        if (affect_sim_type == AffectSimType::SEQUENCE)
        {
            u.monitor_red->reduce();
        }

        // Display the performance (BER and FER) in the terminal
        u.terminal->final_report();

        // Reset the monitor for the next SNR
        m.monitor->reset();

        if (stats_enabeled)
        {
            if (affect_sim_type == AffectSimType::SEQUENCE)
            {
                std::cout << "#" << std::endl;
                spu::tools::Stats::show(u.sequence->get_modules_per_types(), true);
                std::cout << "#\n#" << std::endl;
            }
            else if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
            {
                // Display the statistics of the pipeline stages (if enabled)
                auto stages = u.pipeline->get_stages();
                for (size_t s = 0; s < stages.size(); s++)
                {
                    const int n_threads = stages[s]->get_n_threads();
                    std::cout << "#" << std::endl
                              << "# Pipeline stage " << s << " (" << n_threads << " thread(s)): " << std::endl;
                    spu::tools::Stats::show(stages[s]->get_tasks_per_types(), true);
                }
                std::cout << "#\n#" << std::endl;
            }
        }

        if (affect_sim_type == AffectSimType::SEQUENCE)
        {
            for (auto& mod : u.sequence->get_modules<spu::module::Module>(false))
            {
                for (auto& tsk : mod->tasks)
                {
                    tsk->reset();
                }
            }
        }
        else if (s.sim_type.affectSimType == AffectSimType::PIPELINE)
        {
            for (auto& mod : u.pipeline->get_modules<spu::module::Module>(false))
            {
                for (auto& tsk : mod->tasks)
                {
                    tsk->reset();
                }
            }
        }
    }

    std::cout << "#" << std::endl;
    std::cout << "# End of the simulation" << std::endl;
}
