#!/usr/bin/env python
import os
import sys
import optparse
import platform
import json
import shutil


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

def read_json(fn):
    json_file = ""
    with open(fn) as f:
        json_file = f.read()
    return json.loads(json_file)

def checkgit():
    stat = os.popen('git status --porcelain').read().strip()
    if stat != "":
        print "Missing checkin in", os.getcwd(), "\n" + stat
        exit(1)

def read_config():
    codal = read_json("codal.json")
    targetdir = codal['target']['name']
    target = read_json("libraries/" + targetdir + "/target.json")
    return (codal, targetdir, target)
    
def update():
    (codal, targetdir, target) = read_config()
    dirname = os.getcwd()
    for ln in target['libraries']:
        os.chdir(dirname + "/libraries/" + ln['name'])
        system("git pull")
    os.chdir(dirname + "/libraries/" + targetdir)
    system("git pull")
    os.chdir(dirname)

def lock():
    (codal, targetdir, target) = read_config()
    dirname = os.getcwd()
    for ln in target['libraries']:
        os.chdir(dirname + "/libraries/" + ln['name'])
        checkgit()
        stat = os.popen('git status --porcelain -b').read().strip()
        if "ahead" in stat:
            print "Missing push in", os.getcwd()
            exit(1)
        sha = os.popen('git rev-parse HEAD').read().strip()
        ln['branch'] = sha
        print ln['name'], sha
    os.chdir(dirname + "/libraries/" + targetdir)
    system("git checkout target-locked.json")
    checkgit()
    with open("target-locked.json", "w") as f:
        f.write(json.dumps(target, indent=4))
    if os.popen('git status --porcelain').read().strip() == "":
        print "Was already locked here."
    else:
        system("git commit -am 'Target snapshot.'")
    sha = os.popen('git rev-parse HEAD').read().strip()
    print "\nNew snapshot: " + sha + "\n"
    system("git pull")
    system("git push")
    os.chdir(dirname)
    

parser = optparse.OptionParser()
parser.add_option('-c', '--clean', dest='clean', action="store_true", help='Whether to clean before building. Applicable only to unix based builds.', default=False)
parser.add_option('-t', '--test-platforms', dest='test_platform', action="store_true", help='Whether to clean before building. Applicable only to unix based builds.', default=False)
parser.add_option('-l', '--lock', dest='lock_target', action="store_true", help='Create target-lock.json', default=False)
parser.add_option('-u', '--update', dest='update', action="store_true", help='git pull target and libraries', default=False)

(options, args) = parser.parse_args()

if not os.path.exists("build"):
    os.mkdir("build")

if options.lock_target:
    lock()
    exit(0)

if options.update:
    update()
    exit(0)

# out of source build!
os.chdir("build")



if not options.test_platform:
    build(options.clean)
    exit(0)

test_json = read_json("../utils/targets.json")

for fname in test_json.keys():

    json_obj = test_json[fname]

    # ensure we have a clean build tree.
    os.chdir("..")
    shutil.rmtree('./build')
    os.mkdir("./build")
    os.chdir("./build")

    # clean libs
    if os.path.exists("../libraries"):
        shutil.rmtree('../libraries')

    # configure the target and tests...
    config = {
        "target":json_obj,
        "output":".",
        "application":"libraries/"+json_obj["name"]+"tests/"
    }

    with open("../codal.json", 'w') as codal_json:
        json.dump(config, codal_json)

    build(True)

