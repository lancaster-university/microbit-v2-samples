#!/bin/sh
set -x

# Simply install everything via homebrew
brew tap ArmMbed/homebrew-formulae
brew install arm-none-eabi-gcc
brew install cmake
brew install ninja
brew install srecord
