# Benchmark locks


## Build requirements:

- C++ compiler
- Google bench
- Meson
- Python deps (requirements.txt)


## Build

```
mkdir build
cd build
meson setup --buildtype release ..
ninja
```

## Run

```
# In build/
./spin_bench --benchmark_out=../result.json

# In the root
python3 viz.py
```

