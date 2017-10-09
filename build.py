#!/usr/bin/env python
import os
import sys
import optparse
import platform
import json
import shutil

parser = optparse.OptionParser()
parser.add_option('-c', '--clean', dest='clean', action="store_true", help='Whether to clean before building. Applicable only to unix based builds.', default=False)
parser.add_option('-t', '--test-platforms', dest='test_platform', action="store_true", help='Whether to clean before building. Applicable only to unix based builds.', default=False)

(options, args) = parser.parse_args()

if not os.path.exists("build"):
    os.mkdir("build")

# out of source build!
os.chdir("build")

def system(cmd):
    if os.system(cmd) != 0:
      sys.exit(1)

def build(clean):
    if platform.system() == "Windows":
        # configure
        system("cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -G \"Ninja\"")

        # build
        system("ninja")
    else:
        # configure
        system("cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -G \"Unix Makefiles\"")

        if clean:
            system("make clean")

        # build
        system("make -j 10")

if not options.test_platform:
    build(options.clean)
    exit(0)

print(platform)

for target_fname in os.listdir("../targets"):

    # ensure we have a clean build tree.
    os.chdir("..")
    shutil.rmtree('./build')
    os.mkdir("./build")
    os.chdir("./build")

    # clean libs
    if os.path.exists("../libraries"):
        shutil.rmtree('../libraries')

    # remove extension
    fname = target_fname.split(".")[0]

    # configure the target and tests...
    config = {
        "target":target_fname,
        "output":".",
        "application":"travis_tests/" + fname
    }

    with open("../codal.json", 'w') as codal_json:
        json.dump(config, codal_json)

    build(True)

