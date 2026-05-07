#include "Code/Code_params.hpp"

// init all the variables
CodeParams::CodeParams() {}

size_t
CodeParams::get_k() const
{
    return this->k;
}

size_t
CodeParams::get_n() const
{
    return this->n;
}

size_t
CodeParams::get_r() const
{
    return this->r;
}

float
CodeParams::get_R() const
{
    return this->R;
}
