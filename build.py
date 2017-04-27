import os
import optparse

parser = optparse.OptionParser()
parser.add_option('-c', '--clean', dest='clean', action="store_true", help='Whether to clean before building.', default=False)

(options, args) = parser.parse_args()

if not os.path.exists("build"):
    os.mkdir("build")

# out of source build!
os.chdir("build")

# configure
os.system("cmake ..")

if options.clean:
    os.system("make clean")

# build
os.system("make -j 10")
