/*!
 * \file
 * \brief Class tools::Reporter_noise_CVQKD.
 */
#ifndef REPORTER_NOISE_CVQKD_HPP_
#define REPORTER_NOISE_CVQKD_HPP_
#include <streampu.hpp>

namespace CVQKD
{
namespace tools
{
template<typename R = float>
class Reporter_noise_CVQKD : public spu::tools::Reporter
{
  protected:
    bool binary_channel;
    float linear_ebn0;
    float log_ebn0;
    const float code_rate;
    const float signal_variance;
    float beta;
    group_t noise_group;

  public:
    explicit Reporter_noise_CVQKD(const float& beta,
                                  const float& code_rate,
                                  const bool binary_channel = false,
                                  const float signal_variance = 1.0f);
    virtual ~Reporter_noise_CVQKD() = default;

    void update(const float& new_beta);

    report_t report(bool final = false);
};
}
}

#endif /* REPORTER_NOISE_CVQKD_HPP_ */
