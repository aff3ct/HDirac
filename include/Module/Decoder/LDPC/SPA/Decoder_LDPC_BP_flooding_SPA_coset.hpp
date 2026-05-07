/*!
 * \file
 * \brief Class module::Decoder_LDPC_BP_flooding_SPA_coset.
 */
#ifndef DECODER_LDPC_BP_FLOODING_SPA_COSET_HPP_
#define DECODER_LDPC_BP_FLOODING_SPA_COSET_HPP_

#include <cstdint>
#include <vector>

#include "Module/Decoder/LDPC/BP/Flooding/Decoder_LDPC_BP_flooding.hpp"
#include "Tools/Algo/Matrix/Sparse_matrix/Sparse_matrix.hpp"
#include "Tools/Code/LDPC/Update_rule/SPA/Update_rule_SPA.hpp"

using namespace spu;
using namespace spu::module;

namespace CVQKD
{
namespace tools
{
struct LDPC_syndrome_coset
{
  private:
    static std::map<const aff3ct::tools::Sparse_matrix*, const int*> H_to_syndrome;

  public:
    static inline void set_syndrome(const aff3ct::tools::Sparse_matrix& H, const int* syndrome);

    template<typename R>
    static inline bool check_soft(const std::vector<R>& Y_N, const aff3ct::tools::Sparse_matrix& H);

    template<typename R>
    static inline bool check_soft(const R* Y_N, const aff3ct::tools::Sparse_matrix& H);
};
}

namespace module
{

namespace dec
{
enum class tsk : size_t
{
    set_syndrome,
    SIZE
};

namespace sck
{
enum class set_syndrome : size_t
{
    in_syndrome,
    status
};
}
}

template<typename B = int, typename R = float>
class Decoder_LDPC_BP_flooding_SPA_coset
  : public aff3ct::module::Decoder_LDPC_BP_flooding<B, R, aff3ct::tools::Update_rule_SPA<R>, tools::LDPC_syndrome_coset>
{
  public:
    inline runtime::Task& operator[](const aff3ct::module::dec::tsk t);
    inline runtime::Task& operator[](const CVQKD::module::dec::tsk t);
    inline runtime::Socket& operator[](const CVQKD::module::dec::sck::set_syndrome s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    std::vector<R> values;
    int r;
    const int* syndrome;

    std::vector<R> fwd;
    std::vector<R> bwd;

  public:
    Decoder_LDPC_BP_flooding_SPA_coset(const int K,
                                       const int N,
                                       const int r,
                                       const int n_ite,
                                       const aff3ct::tools::Sparse_matrix& H,
                                       const std::vector<uint32_t>& info_bits_pos,
                                       const bool enable_syndrome = true,
                                       const int syndrome_depth = 1);
    virtual ~Decoder_LDPC_BP_flooding_SPA_coset() = default;
    virtual Decoder_LDPC_BP_flooding_SPA_coset<B, R>* clone() const;

    template<class A = std::allocator<int>>
    void set_syndrome(const std::vector<int, A>& in_syndrome,
                      const int frame_id = -1,
                      const bool managed_memory = true);

    void set_syndrome(const int* in_syndrome, const int frame_id = -1, const bool managed_memory = true);

  protected:
    void _decode_single_ite(const std::vector<R>& msg_var_to_chk, std::vector<R>& msg_chk_to_var);
    void _set_syndrome(const int* in_syndrome, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Decoder/LDPC/SPA/Decoder_LDPC_BP_flooding_SPA_coset.hxx"
#endif

#endif /* DECODER_LDPC_BP_FLOODING_SPA_COSET_HPP_ */