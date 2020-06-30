# Use chocolatey to install Arm GCC, CMake, and Ninja
choco install gcc-arm-embedded
choco install cmake
choco install ninja
# Install srec by downloading it and unzipping into the cwd
C:\msys64\usr\bin\wget.exe -O srecord.zip https://sourceforge.net/projects/srecord/files/srecord-win32/1.64/srecord-1.64-win32.zip/download
Expand-Archive -Force srecord.zip .
