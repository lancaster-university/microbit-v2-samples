# Instructions for building

git clone https://www.github.com/microbit-foundation/codal
cd codal
git checkout nrf52833-mbedos
python build.py

If this fails, for example because you're using GH 2FA and https, then you need to manually fetch all repos:

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
    rm -rf ./build

Verify you are where you think you are and everything's at the right branch
python build.py -s #s is for status

python build.py 

# Troubleshooting

If your build fails with missing toolchain, remove the build dir and try again. This usually happens if the first attempt to run build.py fails and we don't check out all the relevant libraries (often due to GH permissions)

If you get include issues, ensure you're on the right branch for codal-mbedos (listed above)


