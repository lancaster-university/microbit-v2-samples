# Instructions for building

The entry point for micro:bit at this stage is the branch `master`

## System setup

You should have followed the setup required for CODAL in the main readme. One majore difference is that for mb we ship a codal.json that is hardcoded to our target.

For the CODAL build tools to work, you have to have setup GitHub's two-factor authentication to work with git, and generated an access token (this is because the repositories are private.

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

If this fails, for example because you're using GH 2FA and https and don't have a personal access token (as per instructions above), then you need to manually fetch all repos:

```bash
cd libraries/
ls
git clone git@github.com:microbit-foundation/codal-core.git
git clone git@github.com:microbit-foundation/codal-microbit-next.git
git clone git@github.com:microbit-foundation/codal-mbedos.git
git clone git@github.com:microbit-foundation/codal-nrf52.git
ls # You should have the four directories now
cd codal-core/
git checkout nrf52833-mbedos
cd ../
cd codal-mbedos/
git checkout nrf52833-bringup-includes #NOTE THIS IS DIFFERENT TO JOE'S INSTRUCTIONS on 29th OCT as it fixes a Mac build issue found since
cd ../
cd codal-nrf52/
git checkout nrf52833-mbedos
cd ../
cd codal-microbit-next/
git checkout nrf52833-mbedos
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

If you get include issues, ensure you're on the right branch for codal-mbedos (listed above)


