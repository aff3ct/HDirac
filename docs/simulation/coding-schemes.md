# Simulation Coding Schemes

## Sequence Mode

All coding schemes in this project rely on a **coset** decoder, combined with different channel models: BIAWGN and several CVQKD virtual channels $-$ real, complex, quaternion, octonion, and Householder with complexities $O(d^3)$ and $O(d^2)$.

In addition, we include an extra coding scheme based on **syndrome concatenation**, used purely for performance comparison.

The objective is to compare:

- a standard **coset** coding approach (decoding toward a given syndrome), and
- a **coset** coding with **syndrome concatenation**.

A direct comparison with a _classical decoder_ (i.e., decoding toward the zero syndrome) is not performed. This would require the original AFF3CT decoder, which is not suited for low-SNR CV-QKD regimes and does not provide satisfactory performance in terms of efficiency ($\beta$) and frame error rate (FER).

Instead, we use a general-purpose coset decoder that can target any syndrome (including the zero syndrome), and compare **coset** coding with **coset** coding + **syndrome concatenation**.

!!! note
    This additional scheme is NOT intended for practical use, but rather to isolate and evaluate the impact of syndrome concatenation in a controlled setting.

Since the focus is on the decoder behavior (not the channel model), this comparison is implemented only for the **BIAWGN** channel, as extending it to CVQKD virtual channels would not bring additional insight.

## Pipeline Mode

The current pipeline includes:

- A **BIAWGN** channel with a **coset** decoder
- A **BIAWGN** channel with a **coset** decoder + **syndrome concatenation** overlay
- A **$O(d^2)$-Householder-based** CVQKD virtual channel with a **coset** decoder
