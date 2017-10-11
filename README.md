# Codal [![Build Status](https://travis-ci.com/lancaster-university/codal.svg?token=npLd3GxcjQ1s3m7yiTdh&branch=master)](https://travis-ci.com/lancaster-university/codal)

The main repository for the Component Oriented Device Abstraction Layer (codal).

This repository is an empty shell that provides the tooling needed to produce a hex file for a codal device.

## Installation

### Automatic installation.

This software has its grounding on the founding principles of [Yotta](https://www.mbed.com/en/platform/software/mbed-yotta/), the simplest install path would be to install their tools via their handy installer.

### Docker

A [docker image](https://hub.docker.com/r/jamesadevine/codal-toolchains/) is available that contains toolchains used to build codal targets. A wrapper [Dockerfile](https://github.com/lancaster-university/codal-docker) is available that can be used to build your project with ease.

Then follow the build steps listed below.

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

# Configuration

Below is an example of how to configure codal to build the codal-circuit-playground target against your app, which is contained in the folder `application`:

```json
{
    "target":{
        "name":"codal-circuit-playground",
        "url":"https://github.com/lancaster-university/codal-circuit-playground",
        "branch":"master",
        "type":"git"
    },
    "application":"application",
    "output_folder":"."
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
    "application":"application",
    "output_folder":"."
}
```

The above example will be translate `"NUMBER_ONE":1` into: `#define NUMBER_ONE     1` and force include it during compilation.

# Targets

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
