# Available Combinations in the Simulator

!!! info
    We're talking about the simulator here. If HDirac is used as a library, it can support all combinations.

Not every combination of channel, decoder, and execution mode is currently implemented in the simulator.

The following table summarizes the configurations supported by the current version of HDirac.

## Simulator Compatibility Table

| Channel           | Sequence + Coset | Sequence + Coset Synd. | Pipeline + Coset | Pipeline + Coset Synd. |
| ----------------- | :--------------: | :--------------------: | :--------------: | :--------------------: |
| BIAWGN            |         ✅        |            ✅           |         ✅        |            ✅           |
| Real              |         ✅        |            ❌           |         ❌        |            ❌           |
| Complex           |         ✅        |            ❌           |         ❌        |            ❌           |
| Quaternion        |         ✅        |            ❌           |         ❌        |            ❌           |
| Octonion          |         ✅        |            ❌           |         ❌        |            ❌           |
| Householder O(d²) |         ✅        |            ❌           |         ✅        |            ❌           |
| Householder O(d³) |         ✅        |            ❌           |         ❌        |            ❌           |

!!! note
    Supported configurations are derived from the current implementation of the simulator and may evolve in future releases.
