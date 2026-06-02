# HDirac - a High Dimensional Information Reconciliation Application for CV-QKD

<div class="hero-grid" markdown>

<div class="hero-card" markdown>
:material-chart-bell-curve: **Simulator**  
Compare virtual channel methods, dimensions and coding schemes against the ideal BIAWGNC channel.
</div>

<div class="hero-card" markdown>
:material-lightning-bolt: **AFF3CT-based**  
Built on top of AFF3CT, a high-performance framework for channel coding simulation.
</div>

<div class="hero-card" markdown>
:material-puzzle: **Library**  
Use it as a library — plug in custom channels, decoders and modules alongside native AFF3CT components.
</div>

</div>


## What is this project?

This simulator focuses on **multidimensional information reconciliation** for **Continuous-Variable Quantum Key Distribution (CV-QKD)** across arbitrary high dimensions. It emulates the *CV-QKD virtual channel* using various modelling approaches. Acting as a black box, the tool allows users to experiment with different virtual channel methods, dimensional settings, and coding schemes, while comparing the results with those of a Binary Input Additive White Gaussian Noise Channel (BIAWGNC). The latter represents the ideal virtual channel, which is approached as the dimension tends towards infinity.

The codebase is built on top of [AFF3CT](https://aff3ct.github.io/), a high-performance framework dedicated to channel coding simulations.

As well as being used independently, the project can also serve as a library. Users can design custom channels, modify components, or integrate modules, such as channel models, decoders and more, either developed within this project or provided by AFF3CT.

## Publication

HDirac is the open-source simulation framework presented in our paper [Multidimensional Reconciliation in Continuous-Variable QKD: Review, Coding Schemes, and Open Source Simulation](https://arxiv.org/abs/2606.02323). Below is an excerpt from the introduction.

!!! quote ""
    Multidimensional reconciliation has become the method of choice for long-distance CV-QKD. [...] It transforms the physical quantum channel into a virtual channel that approximates a Binary Input Additive White Gaussian Noise (BIAWGN) channel, enabling the use of modern error-correcting codes. [...]

    The core principle involves mapping a subvector of size \(d\) from the quantum channel data to a random vector of the same size used to build the raw secret key. The overall information balance approaches optimality as the dimension \(d\) increases. While algebraic constructions exist for dimensions 1, 2, 4, and 8, higher dimensions require general random transformations.

Our paper focuses on multidimensional reconciliation in high dimensions (beyond dimension 8) and presents an open-source simulation framework closely integrated with [AFF3CT](https://aff3ct.github.io/), enabling reproducible evaluation of advanced reconciliation schemes and LDPC-based coding strategies.

## Quick Navigation

<div class="nav-grid" markdown>

[:material-wrench: **Build & Install**](getting-started/build.md)  
Compile AFF3CT and the project from source.

[:material-code-braces: **LDPC Codes**](getting-started/ldpc-codes.md)  
Download, prepare, and select the right code rate.

[:material-file-plus: **Custom Channels**](getting-started/custom-channel.md)  
Implement your own channel or decoder extension.

[:material-play-circle: **Running Simulations**](simulation/running.md)  
Launch the binary and understand its parameters.

[:material-view-sequential: **Pipeline vs Sequence**](simulation/modes.md)  
Understand the two execution models and when to use each.

[:material-chart-line: **Coding Schemes**](simulation/coding-schemes.md)  
Coset decoding, syndrome concatenation, and channel models.

[:material-function-variant: **Householder Transformation**](concepts/householder-transform.md)  
High-dimension CV-QKD virtual channel concept visually explained.

</div>
