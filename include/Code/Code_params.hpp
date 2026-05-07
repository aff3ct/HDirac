#ifndef CODE_PARAMS_HPP_
#define CODE_PARAMS_HPP_

#include <iostream>
#include <memory>
#include <vector>

class CodeParams
{
  public:
    CodeParams();
    virtual ~CodeParams() = default;

    size_t get_k() const;
    size_t get_n() const;
    size_t get_r() const;
    float get_R() const;

  protected:
    size_t k;                       // number of information bits
    size_t n;                       // codeword size
    size_t r;                       // syndrome size
    float R;                        // code rate (R = k / n)
    std::vector<uint32_t> bits_pos; // bits positions in H matrix
};

#endif