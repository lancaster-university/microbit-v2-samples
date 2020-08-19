# Instructions for building

The entry point for micro:bit at this stage is the branch `master`

## Introduction and Repositories you need access to....

(this is more a note for the Foundation team to ensure we add everyone to the many repositories required)

The codal, developed primarily by Lancaster University is split across a number of target-specific and device specific repositories. For micro:bit builds, you need access to

* microbit-foundaion/codal (MIT)
* microbit-foundation/codal-microbit (MIT)
* microbit-foundation/codal-microbit-nrf5sdk (this is a redistribution of parts of the Nordic SDK, with some minor modifications, under the following licences: https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v12.x.x/doc/12.3.0/licenses.html - it separates modified files from unmodified as separate directories. Everything in the `nRF5SDK` directoruy could be clobbered at any point)

You also need code from the following public repositories
* lancaster-university/codal-core
* lancaster-university/codal-nrf52

The entry point for building is this repsitory, 'codal'. Other repositories will be fetched from here.

You can also use `yotta` to build the relevant code, instead of build.py (which is maintained by the CODAL team across all CODAL targets, whereas yotta is an additional toolchain provided by micro:bit for compatibility with v1 micro:bit). It uses private forks of microbit-foundation/codal-nrf52 and microbit-foundation/codal-core to include the yotta module details. A yotta build script that can be used as instructions can be found [here](https://github.com/microbit-foundation/embedded-dev-env/blob/master/scripts/build_codal_yotta.sh)

## System setup

You should have followed the setup required for CODAL in the main readme. One major difference is that for micro:bit we ship a codal.json that is hardcoded to our target (codal-microbit). At the time of writing, that included an arm-none-eabi-gcc toolchain, CMake, Python 2.7 (reports of success with Python 3 would be welcome).

For the CODAL build tools to work, you have to have setup GitHub's two-factor authentication to work with git, and generated an access token (this is because the repositories are private. It won't be necessary once they are public, and private repositories are not really a target use-case for the build.py tools)

### Generate a personal access token for accessing repositories

1. Create a new personal access token and store it in your password manager (eg 1password)

https://github.com/settings/tokens

You should tick the 'repo' and 'admin:repo_hook' permissions. Save this access token somewhere secure (if on a Mac you can use keychain access if you don't have an existing password manager)

### Tell git to use your credentials for GitHub

(If you really need to skip this step, you can just have github let you login on the command line)

Because the repositories are private, if you don't do this, git will get 404s whenever we try to access the private repos

edit ~/.gitconfig and make sure it contains the following
```gitconfig
[credential "https://github.com"]
        username = YOUR_USERNAME
```
If you're on macOS your life will be better if you also add
```
[credential]
        helper = osxkeychain
```

## Fetch and build codal

```bash
git clone https://www.github.com/microbit-foundation/codal
cd codal
python build.py
```

The output for success will look a bit like this:
```
Scanning dependencies of target MICROBIT_hex
Scanning dependencies of target MICROBIT_bin
[100%] converting to bin file.
[100%] converting to hex file.
[100%] Built target MICROBIT_bin
[100%] Built target MICROBIT_hex
```
And you can flash your micro:bit by copying the hex file to the MICROBIT drive. The default programme responds to button presses.

on macoS:

```
cp MICROBIT.hex /Volumes/MICROBIT
```

If the build fails, for example because you're using GH 2FA and https and don't have a personal access token (as per instructions above), then you need to manually fetch all repos:

**Note: you will need to setup ssh or other authentication for git@ to use this method**

```bash
cd libraries/
ls # check there aren't already stale entries here...
git clone git@github.com:microbit-foundation/codal-microbit.git
git clone git@github.com:microbit-foundation/codal-microbit-nrf5sdk
git clone git@github.com:lancaster-university/codal-core.git
git clone git@github.com:lancaster-university/codal-nrf52.git
ls # You should have the four directories now, and assuming you want to build 'master', no further action is required
# If you'd like to build a specific branch or tag and are using the repos manually, you have to enter each library subdirectory to do so.
# Using build.py with credentials avoids this step
cd codal-core/
git checkout tagname
cd ../
cd codal-mbedos/
git checkout tagname
cd ../
cd codal-nrf52/
git checkout tagname
cd ../
cd codal-microbit-next/
git checkout tagname
cd ../../
```

If your build failed the first time
    `rm -rf ./build`

Verify you are where you think you are and everything's at the right branch

```bash
python build.py -s #s is for status

python build.py 
```

# Troubleshooting

If your build fails with missing toolchain, remove the build dir and try again. This usually happens if the first attempt to run build.py fails and we don't check out all the relevant libraries (often due to GH permissions)


# Raising Issues

Any issues regarding the Micro:Bit should be raised on the [microbit-foundation/codal-microbit](https://github.com/microbit-foundation/codal-microbit) repository.
