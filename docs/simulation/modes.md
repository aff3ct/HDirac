# Pipeline and Sequence Modes

The two main modes of this simulation are **pipeline** and **sequence**, following the terminology of AFF3CT.

## Sequence

A **sequence** consists of chaining together several AFF3CT tasks. A _task_ is an operation performed on data (e.g., modulation) and belongs to a _module_, which can be seen as a class. For example, the modulation task belongs to the modulator module, which may also include other tasks such as demodulation. In a sequence, tasks are executed one after another.

## Pipeline

A **pipeline** can be seen as a sequence split into multiple stages, where each stage contains several tasks. Once stage 0 finishes processing the first batch of data, it can immediately start processing the second batch while stage 1 handles the first one. This overlap enables parallel execution across stages. AFF3CT pipelines allow each stages to be executed in parallel by multiple workers.

## How This Simulation Uses Both

In this simulation, we primarily use **sequences** as described above. However, **pipelines** are used in a more specific way.

Only the decoder task is parallelised (which represents itself a pipeline stage), as it is by far the time-consuming (80% to 99% of the total runtime, depending on the simulation parameters). By doing so, multiple batches of data can be processed simultaneously within the decoder, instead of being blocked behind it in a purely sequential execution.
