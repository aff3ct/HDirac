#include <cassert>
#include <cmath>
#include <iomanip>
#include <ios>
#include <sstream>
#include <streampu.hpp>
#include <tuple>
#include <utility>

#include "Tools/Helper.hpp"
#include "Tools/Reporter/Noise/Reporter_noise_CVQKD.hpp"

using namespace CVQKD;
using namespace CVQKD::tools;

template<typename R>
Reporter_noise_CVQKD<R>::Reporter_noise_CVQKD(const float& beta,
                                              const float& code_rate,
                                              const bool binary_channel,
                                              const float signal_variance)
  : Reporter()
  , binary_channel(binary_channel)
  , linear_ebn0(CVQKD::tools::beta_to_snr(beta, code_rate, binary_channel))
  , log_ebn0(10 * std::log10(linear_ebn0))
  , code_rate(code_rate)
  , signal_variance(signal_variance)
  , beta(beta)
{
    auto& Noise_title = noise_group.first;
    auto& Noise_cols = noise_group.second;

    Noise_title = { "Signal Noise Ratio", "(SNR)", 0 };
    Noise_cols.push_back(std::make_tuple("Eb/N0", "(linear)", 0));
    Noise_cols.push_back(std::make_tuple("Eb/N0", "(dB)", 0));
    Noise_cols.push_back(std::make_tuple("Beta", "(%)", 0));

    this->cols_groups.push_back(noise_group);
}

template<typename R>
void
Reporter_noise_CVQKD<R>::update(const float& new_beta)
{
    this->linear_ebn0 = CVQKD::tools::beta_to_snr(new_beta, this->code_rate, this->binary_channel);
    this->log_ebn0 = 10 * std::log10(linear_ebn0);
    this->beta = new_beta;
}

template<typename R>
spu::tools::Reporter::report_t
Reporter_noise_CVQKD<R>::report(bool /*final*/)
{
    assert(this->cols_groups.size() == 1);

    report_t the_report(this->cols_groups.size());

    auto& noise_report = the_report[0];

    std::stringstream stream;

    stream << std::setprecision(4) << std::fixed << this->linear_ebn0;
    noise_report.push_back(stream.str());
    stream.str("");

    stream << std::setprecision(2) << std::fixed << this->log_ebn0;
    noise_report.push_back(stream.str());
    stream.str("");

    stream << std::setprecision(2) << std::fixed << this->beta * 100;
    noise_report.push_back(stream.str());
    stream.str("");

    return the_report;
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef AFF3CT_MULTI_PREC
template class CVQKD::tools::Reporter_noise_CVQKD<R_32>;
template class CVQKD::tools::Reporter_noise_CVQKD<R_64>;
#else
template class CVQKD::tools::Reporter_noise_CVQKD<R>;
#endif
// ==================================================================================== explicit template instantiation
