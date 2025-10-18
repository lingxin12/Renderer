# Environment

- Ubuntu 22.04
- gcc 13.1.0
- g++ 13.1.0
- GNU Make 4.3
- cuda 12.4
- NVIDIA GeForce RTX 4090

If you run the code on win11, you can use `MinGW` which g++ version is 15.2.0. Please ensure that the compiler version can stably support c++23.

# Demo

```bash
# Rasterization
cd Rasterization
mkdir build/obj
make
./t # or ./t --cuda

# Ray-Tracing
cd Ray-Tracing
g++ main.cpp -o t
./t
```

# Future work

- MSAA or TAA
- Ray-Tracing on cuda
- Rasterization Renderer with AABB and barycentric rasterization
- Improve DDA algorithm on cuda
- ......

