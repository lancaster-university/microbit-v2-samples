# Codal [![Build Status](https://travis-ci.org/lancaster-university/codal.svg?branch=master)](https://travis-ci.org/lancaster-university/codal)

The main repository for the Component Oriented Device Abstraction Layer (CODAL).

This repository is an empty shell that provides the tooling needed to produce a bin/hex/uf2 file for a CODAL device.

## Raising Issues

Any issues regarding the micro:bit are gathered on the [microbit-foundation/codal-microbit](https://github.com/microbit-foundation/codal-microbit) repository. Please raise yours on that repository as well.

## Installation

### Automatic installation.

This software has its grounding on the founding principles of [Yotta](https://www.mbed.com/en/platform/software/mbed-yotta/), the simplest install path would be to install their tools via their handy installer.

### Docker

A [docker image](https://hub.docker.com/r/jamesadevine/codal-toolchains/) is available that contains toolchains used to build codal targets. A wrapper [Dockerfile](https://github.com/lancaster-university/codal-docker) is available that can be used to build your project with ease.

Then follow the build steps listed below.

### Manual installation

1. Install `git`, ensure it is available on your platforms path.
2. Install the `arm-none-eabi-*` command line utilities for ARM based devices and/or `avr-gcc`, `avr-binutils`, `avr-libc` for AVR based devices, ensure they are available on your platforms path.
3. Install [CMake](https://cmake.org)(Cross platform make), this is the entirety of the build system.
    5. If on Windows, install ninja.
4. Install `Python 2.7` (if you are unfamiliar with CMake), python scripts are used to simplify the build process.
5. Clone this repository

# Building
- Generate or create a `codal.json` file
    - `python build.py ls` lists all available targets
    - `python build.py <target-name>` generates a codal.json file for a given target
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

# Configuration

Below is an example of how to configure codal to build the [codal-circuit-playground](https://github.com/lancaster-university/codal-circuit-playground) target, example applications will automatically be loaded into the "source" folder:

```json
{
    "target":{
        "name":"codal-circuit-playground",
        "url":"https://github.com/lancaster-university/codal-circuit-playground",
        "branch":"master",
        "type":"git"
    }
}
```

For more targets, read the targets section below.

## Advanced

If you would like to override or define any additional configuration options (`#define's`) that are used by the supporting libraries, the codal build system allows the addition of a config field in `codal.json`:

```json
{
    "target":{
        "name":"codal-circuit-playground",
        "url":"https://github.com/lancaster-university/codal-circuit-playground",
        "branch":"master",
        "type":"git"
    },
    "config":{
        "NUMBER_ONE":1
    },
    "application":"source",
    "output_folder":"."
}
```

The above example will be translate `"NUMBER_ONE":1` into: `#define NUMBER_ONE     1` and force include it during compilation. You can also specify alternate application or output folders.

# Targets

To obtain a full list of targets type:

```
python build.py ls
```

To generate the `codal.json` for a target listed by the ls command, please run:

```
python build.py <target-name>
```

Please note you may need to remove the libraries folder if your previous build relied on similar dependencies.

## Arduino Uno

This target specifies the arduino uno which is driven by an atmega328p.

### codal.json specification
```json
"target":{
    "name":"codal-arduino-uno",
    "url":"https://github.com/lancaster-university/codal-arduino-uno",
    "branch":"master",
    "type":"git"
}
```
This target depends on:
* [codal-core](https://github.com/lancaster-university/codal-core) provides the core CODAL abstractions
* [codal-atmega328p](https://github.com/lancaster-university/codal-atmega328p) implements basic CODAL components (I2C, Pin, Serial, Timer)

## BrainPad

This target specifies the BrainPad which is driven by a STM32F.

### codal.json specification
```json
"target":{
    "name":"codal-brainpad",
    "url":"https://github.com/lancaster-university/codal-brainpad",
    "branch":"master",
    "type":"git"
}
```
This target depends on:
* [codal-core](https://github.com/lancaster-university/codal-core) provides the core CODAL abstractions
* [codal-mbedos](https://github.com/lancaster-university/codal-mbed) implements required CODAL basic components (Timer, Serial, Pin, I2C, ...) using Mbed

## Circuit Playground

This target specifies the circuit playground which is driven by a SAMD21.

### codal.json specification
```json
"target":{
    "name":"codal-circuit-playground",
    "url":"https://github.com/lancaster-university/codal-circuit-playground",
    "branch":"master",
    "type":"git"
}
```
This target depends on:
* [codal-core](https://github.com/lancaster-university/codal-core) provides the core CODAL abstractions
* [codal-mbed](https://github.com/lancaster-university/codal-mbed) implements required CODAL basic components (Timer, Serial, Pin, I2C, ...) using Mbed
* [codal-samd21](https://github.com/lancaster-university/codal-samd21) implements SAMD21-specific components (such as USB)
* [mbed-classic](https://github.com/lancaster-university/mbed-classic) is a fork of mbed, used by codal-mbed
