# Benchmark locks


## Build requirements:

- C++ compiler
- CMake
- Python deps (requirements.txt)


## Build

```
git submodule update --init --recursive
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. 
ninja
```

## Run

```
# In build/
./spin_bench --benchmark_out=../result.json

# In the root
python3 viz.py
```

