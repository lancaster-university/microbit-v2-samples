# Codal

The main repository for the Component Oriented Device Abstraction Layer (codal).

This repository is an empty shell that provides the tooling needed to produce a hex file for a codal device.

## Supported targets
- Adafruit Circuitplayground (SAMD21G18A) - [codal-samd21-mbed](https://github.com/lancaster-university/codal-samd21-mbed) Provides mbed drivers & support layer for samd21 based devices.

## Libraries:
- [codal-core](https://github.com/lancaster-university/codal-core) - A collection of components that form the basis of the codal runtime.
- [codal-mbed](https://github.com/lancaster-university/codal-mbed) - A collection of drivers that utilise mbed-classic.
- [mbed-classic](https://github.com/lancaster-university/mbed-classic) - A platform that provides abstraction for a number of different platforms.

## Installation

1. Install `git`, ensure it is available on your platforms path.
- Install the `arm-none-eabi-*` command line utilities, ensure it is available on your platforms path.
- Install [CMake](https://cmake.org)(Cross platform make), this is the entirety of the build system.
- Install `Python 2.7` (if you are unfamiliar with CMake), python scripts are used to simplify the build process.
- Clone this repository

# Building
- In the root of this repository type `python build.py`
    - If you are not using python:
        1. In the root of the repository make a build folder.
        2. `cd build`
        3. `cmake ..`
        4. `make`
- The hex file will be placed at the location specified by `codal.json`, by default this is the root.
