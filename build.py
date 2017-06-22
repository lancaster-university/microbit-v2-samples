#!/usr/bin/env python
import os
import sys
import optparse
import platform

parser = optparse.OptionParser()
parser.add_option('-c', '--clean', dest='clean', action="store_true", help='Whether to clean before building. Applicable only to unix based builds.', default=False)

(options, args) = parser.parse_args()

if not os.path.exists("build"):
    os.mkdir("build")

# out of source build!
os.chdir("build")

def system(cmd):
    if os.system(cmd) != 0:
      sys.exit(1)

if platform.system() == "Windows":
    # configure
    system("cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -G \"Ninja\"")

    # build
    system("ninja")
else:
    # configure
    system("cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -G \"Unix Makefiles\"")

    if options.clean:
        system("make clean")

    # build
    system("make -j 10")
