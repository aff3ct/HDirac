# Running the Simulation

As mentioned earlier, the compiled simulation binary is located at `build/bin/HDirac`.
To run the simulation, an LDPC code in the form of a `.bsparse` file must be provided. For more details, please refer to the [LDPC Codes](../getting-started/ldpc-codes.md) section.

The available parameters can be displayed and usage instructions by executing

```bash
./bin/HDirac -h
```

from the `build/` directory.

!!! note
    Pressing `Ctrl + C` twice quickly will exit the simulation.  
    Pressing `Ctrl + C` once will only skip the current simulation round.

 
## Structure Parameters
 
#### LDPC Code File `--file` <span class="badge-required">REQUIRED</span> { data-toc-label="LDPC Code File" }

| | |
|---|---|
| **Type** | path |
| **Example** | `--file codes/n200000r0p1_cil.bsparse` |
 
Path to the LDPC code file in `.bsparse` format. See [LDPC Codes](../getting-started/ldpc-codes.md) for how to generate this file.
 
---
 
#### Channel Type `--channel` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Channel Type" }
 
| | |
|---|---|
| **Type** | `string` |
| **Default** | `householder_od2` |
| **Allowed values** | `biawgn` `real` `complex` `quaternion` `octonion` `householder_od2` `householder_od3` |
| **Example** | `--channel octonion` |
 
Select the virtual channel model to simulate.
 
| Value | Description |
|---|---|
| `biawgn` | Binary input additive white Gaussian noise $-$ ideal limit as \(d \to \infty\) |
| `real` | CVQKD virtual channel, real-valued (\(d=1\)) |
| `complex` | CVQKD virtual channel, complex (\(d=2\)) |
| `quaternion` | CVQKD virtual channel, quaternion (\(d=4\)) |
| `octonion` | CVQKD virtual channel, octonion (\(d=8\)) |
| `householder_od2` | Householder-based virtual channel with \(O(d^2)\) complexity |
| `householder_od3` | Householder-based virtual channel with \(O(d^3)\) complexity |
 
!!! tip "Aliases"
    `quat` = `quaternion`  
    `oct`  = `octonion`  
    `hod2` = `householder_od2`  
    `hod3` = `householder_od3`  
 
---
 
#### Coding sheme `--decoder` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Coding sheme" }
 
| | |
|---|---|
| **Type** | `string` |
| **Default** | `coset` |
| **Allowed values** | `coset` `coset_synd` |
| **Example** | `--decoder coset` |
 
Select the coding scheme.
 
| Value | Description |
|---|---|
| `coset` | Standard coset decoder $-$ decodes toward a given syndrome |
| `coset_synd` | Coset decoder with syndrome concatenation $-$ for performance comparison only |
 
!!! tip "Alias"
    `cs` = `coset_synd`
 
!!! warning
    `coset_synd` is not intended for practical use. It is only available with `biawgn`,  
    regardless of the mode (`sequence` or `pipeline`).
 
---
 
#### Execution Mode `--mode` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Execution Mode" }
 
| | |
|---|---|
| **Type** | text |
| **Default** | `pipeline` |
| **Allowed values** | `sequence` `pipeline` |
| **Example** | `--mode pipeline` |
 
Select the execution mode. See [Pipeline & Sequence](modes.md) for a detailed explanation.
 
| Value | Description |
|---|---|
| `sequence` | Tasks are chained and executed one after another |
| `pipeline` | The decoder stage is parallelised — multiple batches processed simultaneously |
 
!!! warning
    In `pipeline` mode, `coset` is only available with `biawgn` and `householder_od2`.  
    `coset_synd` is only available with `biawgn`.
 
---

#### Dimension `--d` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Dimension" }

 
| | |
|---|---|
| **Type** | integer |
| **Default** | `64` |
| **Example** | `--d 128` |

!!! note
    The dimension \(d\) of the CVQKD virtual channel. Required to be `> 0` for `householder_od2` and `householder_od3` channels. Must divide the frame length \(n\) evenly — see [Choose of the Code Rate](../getting-started/ldpc-codes.md#choose-of-the-code-rate).
 
 
## Simulation Parameters

#### Beta settings

##### Minimum Beta Value `--beta-min` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Minimum Beta Value" }

| | |
|---|---|
| **Type** | float |
| **Default** | `0.85` |
| **Constraint** | \( < \) `--beta-max`, \(\geq 0.0\) |
| **Example** | `--beta-min 0.9` |
 
Define the minimum \(\beta\) value.

##### Maximum Beta Value `--beta-max` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Maximum Beta Value" }

| | |
|---|---|
| **Type** | float |
| **Default** | `1.0` |
| **Constraint** | \( > \) `--beta-min`, \( \leq 1.0 \) |
| **Example** | `--beta-max 0.98` |
 
Define the maximum \( \beta \) value.

##### Beta  `--beta-step` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Minimum Beta Value" }

| | |
|---|---|
| **Type** | float |
| **Default** | `0.01` |
| **Constraint** | \(> 0\) |
| **Example** | `--beta-step 0.02` |
 
Define the sweep range of reconciliation efficiency \(\beta\). The simulation runs from `--beta-min` to `--beta-max` with increment `--beta-step`.


 
---
 
#### Maximum Error Frames `--max-error-frame` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Maximum Error Frames" }
 
| | |
|---|---|
| **Type** | float |
| **Default** | `100` |
| **Example** | `--max-error-frame 10` |
 
Stop the simulation for a given \(\beta\) once this number of frame errors has been reached.
 
---
 
#### Maximum Frames `--max-frame` <span class="badge-optional">OPTIONAL</span> { data-toc-label="Maximum Frames" }
 
| | |
|---|---|
| **Type** | integer |
| **Default** | `1000` |
| **Example** | `--max-frame 250` |
 
Maximum number of frames to simulate per \(\beta\) point, regardless of error count.


## Other Options
 
#### Statistics `--stats`  <span class="badge-optional">OPTIONAL</span> { data-toc-label="Statistics" }
 
Print detailed per-task timing statistics at the end of the simulation. Or per-stage if a pipeline is used.
 
---
 
#### Help `--help`, `--h`, `-h`  <span class="badge-help">HELP</span> { data-toc-label="Help" }
 
Print the help message and exit.
