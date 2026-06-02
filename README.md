# HDirac - a High Dimensional Information Reconciliation Application for CV-QKD

Simulator and library for **Continuous-Variable Quantum Key Distribution (CV-QKD)** multidimensional information reconciliation.


## Publication

HDirac is the open-source simulation framework presented in our paper **[Multidimensional Reconciliation in Continuous-Variable QKD: Review, Coding Schemes, and Open Source Simulation](https://arxiv.org/abs/2606.02323)**. Below is an excerpt from the introduction.

>Multidimensional reconciliation has become the method of choice for long-distance CV-QKD. [...] Multidimensional reconciliation transforms the physical quantum channel into a virtual channel that approximates a Binary Input Additive White Gaussian Noise (BIAWGN) channel, enabling the use of modern error-correcting codes. [...]
>
> The core principle involves mapping a subvector of size \(d\) from the quantum channel data to a random vector of the same size used to build the raw secret key. The overall information balance approaches optimality as the dimension \(d\) increases. While algebraic constructions exist for dimensions 1, 2, 4, and 8, higher dimensions require general random transformations.

Our paper focuses on multidimensional reconciliation in high dimensions (beyond dimension 8) and presents an open-source simulation framework closely integrated with **[AFF3CT](https://aff3ct.github.io/)**, enabling reproducible evaluation of advanced reconciliation schemes and LDPC-based coding strategies.


## Getting Started

Follow the **[build instructions](https://aff3ct.github.io/HDirac/getting-started/build)**.


## Documentation

Full documentation is available **[here](https://aff3ct.github.io/HDirac)**.


## License

The project is licensed under the MIT license.
