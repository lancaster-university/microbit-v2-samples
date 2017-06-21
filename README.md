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

### Automatic installation.

This software has its grounding on the founding principles of [Yotta](https://www.mbed.com/en/platform/software/mbed-yotta/), the simplest install path would be to install their tools via their handy installer.

### Manual installation

1. Install `git`, ensure it is available on your platforms path.
2. Install the `arm-none-eabi-*` command line utilities, ensure it is available on your platforms path.
3. Install [CMake](https://cmake.org)(Cross platform make), this is the entirety of the build system.
    5. If on Windows, install ninja.
4. Install `Python 2.7` (if you are unfamiliar with CMake), python scripts are used to simplify the build process.
5. Clone this repository




# Building
- In the root of this repository type `python build.py` the `-c` option cleans before building.
    - If you are not using python:
        - Windows:
            1. In the root of the repository make a build folder.
            2. `cd build`
            3. `cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo`
            4. `ninja`
        - Mac:
            1. In the root of the repository make a build folder.
            2. `cd build`
            3. `cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo`
            4. `make`

- The hex file will be placed at the location specified by `codal.json`, by default this is the root.
