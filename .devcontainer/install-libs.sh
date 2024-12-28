#!/bin/bash
export DEBIAN_FRONTEND=noninteractive
export TZ="Europe/Berlin"

apt-get update -y && apt-get upgrade -y

# Install CUDA
apt-get install -y nvidia-cuda-toolkit

# Install GTest
apt-get install -y libgtest-dev
cd /usr/src/googletest/googletest
mkdir build
cd build
cmake ..
make -j
cp libgtest* /usr/lib/
cd ..
rm -rf build

# Install InfiniBand libraries
apt-get install -y libibverbs-dev

# Install NUMA
apt-get install -y libnuma-dev