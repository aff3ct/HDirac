# LDPC Codes

## Source Repository

The LDPC codes used in our examples are publicly available in the [repository by Erdem Ray Çil and Laurent Schmalen](https://github.com/erdemeray/IR_for_CVQKD).

Before using these codes, please review the license of the original repository.
To reproduce our simulations, download the codes and build with _create_bsparse_from_cil_ a **.bsparse** file with the desired code rate and quasi-cyclic expansion factor.

!!! tip
    See _install_cil.sh_ for an example of a script that can be used to obtain the initial matrices.

See the [create_bsparse_from_cil reference](create-bsparse.md) for full usage.


## Choose of the Code Rate

For a binary input with additive white Gaussian noise channel (BIAWGNC), the code rate can be any value. For a CVQKD virtual channel, the length of a frame (\(n\)) must be divisible by the dimension (\(d\)). Since the matrices used here have a fixed initial information size of \(k = 20000\), the rate \(R = \frac{k}{n}\) must be chosen carefully so that \(n \equiv 0 \pmod{d}\).
