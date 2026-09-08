# microbit-v2-samples

[![Native Build Status](https://github.com/lancaster-university/microbit-v2-samples/actions/workflows/build.yml/badge.svg)](https://github.com/lancaster-university/microbit-v2-samples/actions/workflows/build.yml) [![Docker Build Status](https://github.com/lancaster-university/microbit-v2-samples/actions/workflows/docker-image.yml/badge.svg)](https://github.com/lancaster-university/microbit-v2-samples/actions/workflows/docker-image.yml)

This repository provides the necessary tooling to compile a C/C++ CODAL program for the micro:bit V2 and generate a HEX file that can be downloaded to the device.

## Raising Issues
Any issues regarding the micro:bit are gathered on the [lancaster-university/codal-microbit-v2](https://github.com/lancaster-university/codal-microbit-v2) repository. Please raise yours there too.

# Installation
You need some open source pre-requisites to build this repo. You can either install these tools yourself, or use the docker image provided below.

- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- [Git](https://git-scm.com)
- [CMake](https://cmake.org/download/)
- [Python 3](https://www.python.org/downloads/)

We use Ubuntu Linux for most of our tests. You can also install these tools easily through the package manager:

```
    sudo apt install gcc
    sudo apt install git
    sudo apt install cmake
    sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

## Yotta
For backwards compatibility with [microbit-samples](https://github.com/lancaster-university/microbit-samples) users, we also provide a yotta target for this repository.

## Docker
You can use the [Dockerfile](https://github.com/lancaster-university/microbit-v2-samples/blob/master/Dockerfile) provided to build the samples, or your own project sources, without installing additional dependencies.

Run the following command to build the image locally; the .bin and .hex files from a successful compile will be placed in a new `out/` directory:

```
    docker build -t microbit-tools --output out .
```

To omit the final output stage (for CI, for example) run without the `--output` arguments:

```
    docker build -t microbit-tools .
```

## Nix flake
A hashed and locked dev environment is provided via a nix flake.
This avoids polluting your system environment with tooling required for a particular project.

Install nix by following the instructions [here](https://nix.dev/install-nix.html).

To set up and enter the dev environment, run `nix develop`.
Optionally, if you want to use `ninja` to build, run `nix develop .#ninja` to include this package in the environment.
Ninja can help speed up build times on slow machines.
While inside the environment, all the build tooling will be available on the command line with the specified versions.
Pressing Ctrl-D will exit the environment and the tooling will no longer be accessible.

You can also optionally automate this process by installing [direnv](https://direnv.net/docs/installation.html).
Having done this, run
```sh
echo "use flake" >> .envrc
direnv allow
```
Once this is done, entering the repo directory will automatically activate the devshell.
Navigating out of the directory will deactivate it.
This comes with the added benefit of keeping your shell profile active.
As above, if you wish to use `ninja`, replace the echo call with `echo "use flake .#ninja" >> .envrc`.

# Building
- Clone this repository
- In the root of this repository type `python build.py`
- The hex file will be built `MICROBIT.hex` and placed in the root folder.

# Developing
You will find a simple main.cpp in the `source` folder which you can edit. CODAL will also compile any other C/C++ header files our source files with the extension `.h .c .cpp` it finds in the source folder.

The `samples` folder contains a number of simple sample programs that utilise you may find useful.

## Developer codal.json

There is an example `coda.dev.json` file which enables "developer builds" (clones dependencies from the latest commits, instead of the commits locked in the `codal-microbit-v2` tag), and adds extra CODAL flags that enable debug data to be printed to serial.
To use it, simply copy the additional json entries into your `codal.json` file, or you can replace the file completely (`mv coda.dev.json codal.json`).

# Debugging
## VS Code
If you are using Visual Studio Code, there is a working debugging environment already set up for you, allowing you to set breakpoints and observe the micro:bit's memory. To get it working, follow these steps:

1. Install either [OpenOCD](http://openocd.org) or [PyOCD](https://github.com/pyocd/pyOCD).
2. Install the [`marus25.cortex-debug` VS Code extension](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug).
3. Build your program.
4. Click the Run and Debug option in the toolbar.
5. Two debugging options are provided: one for OpenOCD, and one for PyOCD. Select the correct one depending on the debugger you installed.

This should launch the debugging environment for you. To set breakpoints, you can click to the left of the line number of where you want to stop.

## gdb
You can also use `gdb` for debugging.
If you are not using the nix devshell, install OpenOCD with the link in the previous section.
If you are using the devshell, it will already be available.
1. Build your program
2. Run `openocd -f interface/cmsis-dap.cfg -f target/nrf52.cfg`
3. In a second terminal, run
```
    arm-none-eabi-gdb build/MICROBIT -tui \
    -ex "target extended-remote :3333" \
    -ex "monitor reset halt" \
    -ex "load"
```
This will reset and halt the MCU and flash your binary onto it.
Once the binary is flashed, you can re-attach with the same command, just removing the last two `-ex` arguments.

# Compatibility
This repository is designed to follow the principles and APIs developed for the first version of the micro:bit. We have also included a compatibility layer so that the vast majority of C/C++ programs built using [microbit-dal](https://www.github.com/lancaster-university/microbit-dal) will operate with few changes.

# Documentation
API documentation is embedded in the code using doxygen. We will produce integrated web-based documentation soon.
