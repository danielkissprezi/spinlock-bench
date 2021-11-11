# Benchmark locks


## Build requirements:

- Git [Optional, but strongly recommended]
- C++ compiler
- CMake
- Ninja [Optional, but strongly recommended]
- [Optional] Python deps `pip3 install requirements.txt`


## TL;DR

```
git clone https://github.com/danielkissprezi/spinlock-bench
cd spinlock-bench
git submodule update --init --recursive
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. 
cmake --build .
./spin_bench --benchmark_out=result.json
```
