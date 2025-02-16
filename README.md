# Jacobian Chaining

## Build

### Requirements

- **C++23 Compiler / C++23 Standard Library**

   Tested on Linux with:
   - GCC 14.1.0
   - Clang 17.1.6 (libc++)
   - Intel icx 2024.1.0
   - AMD aocc 5.0.0

   Tested on macOS with:
   - AppleClang 16.0.0

   Tested on Windows with:
   - Visual Studio 2022 (msvc 17.x)

- **CMake >= 3.25.0**

   On Windows we need at least CMake 3.30.0 if OpenMP is enabled.

- **Python >= 3.7** (only to generate plots)

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