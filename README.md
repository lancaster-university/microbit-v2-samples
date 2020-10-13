# microbit-v2-samples
This repository is provides the tooling needed to compile a C/C++ CODAL program for the micro:bit v2 and produce a HEX file that can be downloaded to the device.

## Raising Issues
Any issues regarding the micro:bit are gathered on the [lancaster-university/codal-microbit-v2](https://github.com/lancaster-university/codal-microbit-v2) repository. Please raise yours there too.

## Installation
You need some open source pre-requisites to build this repo. You can either install these tools yourself, or use the docker image provided below.

  - [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
  - [Github desktop](https://desktop.github.com/)
  - [CMake](https://cmake.org/download/)
  - [Python 3](https://www.python.org/downloads/)

We use Ubuntu Linux for most of our tests. You can also install these tools easily through the package manager:

```
    sudo apt install gcc
    sudo apt install git
    sudo apt install cmake
    sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

### Docker
If you prefer, a [docker image](https://hub.docker.com/r/jamesadevine/codal-toolchains/) is available that contains the necessary toolchains. A wrapper [Dockerfile](https://github.com/lancaster-university/codal-docker) is also available that can be used to build your project with ease.

### Yotta
For backwards compatibility with [microbit-samples](https://www.github.com/lancaster-univrsity/microbit-samples) users, we also provide a yotta target for this repository.

# Building
- Clone this repository
- In the root of this repository type `python build.py`
- The hex file will be built `MICROBIT.HEX` and placed in the root folder.

# Developing
You will find a simple main.cpp in the `source` folder which you can edit. CODAL will also compile any other C/C++ header files our source files with the extension `.h .c .cpp` it finds in the source folder.

The `samples` folder contains a number of simple sample programs that utilise you may find useful.
