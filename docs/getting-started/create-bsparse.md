# Getting .bsparse from Çil matrices

`create_bsparse_from_cil` is a build tool that converts a CIL matrix file from the
[Çil & Schmalen repository](https://github.com/erdemeray/IR_for_CVQKD) into a `.bsparse`
file ready to be consumed by the simulator. It optionally applies a target code rate and
a quasi-cyclic (QC) expansion.

The binary is built alongside the main simulator — after a successful build it is available at
`build/bin/create_bsparse_from_cil`.


## Parameters

#### Matrix path `-p` <span class="badge-required">REQUIRED</span> { data-toc-label="Matrix Path" }
| | |
| :--- | :--- |
| **Type** | <span class="lozenge">`path`</span> |
| **Example** | <span class="lozenge">`-p ../data/cil_matrices/`</span> |

Path to the directory containing the input CIL matrix file.

---

#### Matrix filename `-f` <span class="badge-required">REQUIRED</span> { data-toc-label="Matrix Filename" }
| | |
| :--- | :--- |
| **Type** | <span class="lozenge">`string`</span> |
| **Example** | <span class="lozenge">`-f R_0p2_R_0p01_RA_SPPCOM_2`</span> |

Filename of the CIL matrix (without extension).

---

#### Choose of the Code Rate `-r` <span class="badge-required">REQUIRED</span> { data-toc-label="Choose of the Code Rate" }
| | |
| :--- | :--- |
| **Type** | <span class="lozenge">`float`</span> |
| **Example** | <span class="lozenge">`-r 0.9`</span> |

Target code rate. The tool will augment the base matrix to reach this rate. 

!!! info "Note"
    Must satisfy the $n \equiv 0 \pmod{d}$ constraint for CVQKD channels.

---

#### Output Directory `-o` <span class="badge-required">REQUIRED</span> { data-toc-label="Output Directory" }
| | |
| :--- | :--- |
| **Type** | <span class="lozenge">`path`</span> |
| **Example** | <span class="lozenge">`-o ../data/h_matrices/`</span> |

Path to the output directory where the `.bsparse` file will be written.

---

#### Output Filename `-n` <span class="badge-required">REQUIRED</span> { data-toc-label="Output Filename" }
| | |
| :--- | :--- |
| **Type** | <span class="lozenge">`string`</span> |
| **Example** | <span class="lozenge">`-n n200000r0p1_cil_RA_SPPCOM_2.bsparse`</span> |

Filename of the output `.bsparse` file.

---

#### QC Expansion `-e` <span class="badge-optional">OPTIONAL</span> { data-toc-label="QC Expansion" }
| | |
| :--- | :--- |
| **Type** | <span class="lozenge">`integer`</span> |
| **Default** | <span class="lozenge">`1`</span> |
| **Example** | <span class="lozenge">`-e 64`</span> |

Quasi-cyclic (QC) expansion factor. When greater than 1, the tool applies a QC lifting.

---

#### Display Help `-h` <span class="badge-help">HELP</span> { data-toc-label="Display Help" }
| | |
| :--- | :--- |
| **Usage** | <span class="lozenge">`./bin/HDirac -h`</span> |

Print the help message and exit.


## Example Command

!!! Example
    ```
    ./bin/create_bsparse_from_cil \
        -p ../data/cil_matrices/ \
        -f R_0p2_R_0p01_RA_SPPCOM_2 \
        -r 0.1 \
        -e 2 \
        -o ../data/h_matrices/ \
        -n q2_n400000r0p1_cil.bsparse
    ```
