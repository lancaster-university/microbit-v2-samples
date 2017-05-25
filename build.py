import os
import optparse
import platform

parser = optparse.OptionParser()
parser.add_option('-c', '--clean', dest='clean', action="store_true", help='Whether to clean before building. Applicable only to unix based builds.', default=False)

(options, args) = parser.parse_args()

if not os.path.exists("build"):
    os.mkdir("build")

# out of source build!
os.chdir("build")

if platform.system() == "Windows":
    # configure
    os.system("cmake .. -G \"Ninja\"")

    # build
    os.system("ninja")
else:
    # configure
    os.system("cmake .. -G \"Unix Makefiles\"")

    if options.clean:
        os.system("make clean")

    # build
    os.system("make -j 10")
