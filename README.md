# Jacobian Chaining

This repository contains a reference implementation which solves various versions of the Jacobian Chain Bracketing Problem. The solver can handle matrix-free, limited-memory and scheduled versions of the problem. There is a dynamic programming algorithm which will solve the computationally tractable problems and will act as aheuristic for the NP-complete scheduled variant. A second branch & bound algorithm can be used to find the global optimum for all variants.

## Build

### Requirements

- **C++23 Compiler / C++23 Standard Library**

   Tested on Linux with:
   - GCC 14.1.0
   - Clang 18.1.8 (libc++)
   - Intel icx 2024.1.0
   - AMD aocc 5.0.0

   Tested on macOS with:
   - AppleClang 16.0.0

   Tested on Windows with:
   - Visual Studio 2022 (msvc 17.x)

- **CMake >= 3.25.0**

   On Windows we need at least CMake 3.30.0 if OpenMP is enabled.

- **Python >= 3.9** (only to generate plots)

   Packages:
   - argparse
   - matplotlib
   - pandas
   - seaborn

### Commands

```shell
mkdir build
cmake -B build -S . -DCMAKE_CXX_COMPILER=<replace-me>
cmake --build build

./build/bin/jcdp ./additionals/configs/config.in
```

### Options

```shell
cmake -DJCDP_USE_OPENMP=<ON|OFF>
export OMP_NUM_THREADS=<replace-me>
```

## Docker container

We also provide a Docker file which will create an Ubuntu 24.10 image with all necessary tools and automatically build the project. To build the Docker image run the following command from the root directory:

```shell
docker build --file additionals/docker/Dockerfile . --tag jcdp
```

In the docker image the executable are located at `/app/jcdp` and `/app/jcdp_batch`. The config files are in `/app/configs` and the plotting script is at `/app/generate_plots.py`. To directly run the solver, run:

```shell
docker run -it jcdp /app/jcdp /app/configs/config.in
```

Alternatively, just run `docker run -it jcdp` and work from inside the container.

## Config files

The config files are checked for the following key-value pairs:

- `length <q>`  
   Length of the Jacobian chains.

- `size_range <lower bound> <upper bound>`  
   Range for the input $n_i$ and output sizes $m_i$.

- `dag_size_range <lower bound> <upper bound>`  
   Range for the size of the elemental DAGs $|E_i|$.

- `available_threads <m>`  
   Number of available machines / threads for scheduling. $m=0$ indicates infinite threads (unlimited parallelism).

- `available_memory <M>`  
   Memory limit per machine. $\bar{M} = 0$ indicates infinite memory.

- `matrix_free <0/1>`  
   Flag that enables matrix-free variant of the Jacobian Chain Bracketing Problem.

- `time_to_solve <s>`  
   Time limit in seconds for the runtime of the Branch & Bound solvers.

- `seed <rng>`  
   Seed for the random number generator in the Jabobian chain generator for reproducibility.

- `amount <n>`  
   Number of chains to generate and solve. Only used by `jcdp_batch`.
