#!/bin/sh
set -x

# Add apt repositories for Arm GCC and CMake
sudo add-apt-repository -y ppa:team-gcc-arm-embedded/ppa
sudo add-apt-repository -y ppa:adrozdoff/cmake
sudo apt-get update -qq
# Simply install everything via apt
sudo apt-get install -y gcc-arm-embedded
sudo apt-get install -y cmake
sudo apt-get install -y ninja-build
sudo apt-get install -y srecord
