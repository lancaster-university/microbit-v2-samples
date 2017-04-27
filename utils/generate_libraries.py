import os
import git
from git import Actor
import optparse
import fnmatch
import glob
import shutil
import ntpath

def copytree(src, dst, symlinks=False, ignore=None):
    if not os.path.exists(dst):
        os.makedirs(dst)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            copytree(s, d, symlinks, ignore)
        else:
            if not os.path.exists(d) or os.stat(s).st_mtime - os.stat(d).st_mtime > 1:
                shutil.copy2(s, d)

def path_leaf(path):
    head, tail = ntpath.split(path)
    return tail or ntpath.basename(head)

def recursive_glob(treeroot, pattern):
    results = []
    for base, dirs, files in os.walk(treeroot):
        goodfiles = fnmatch.filter(files, pattern)
        results.extend(os.path.join(base, f) for f in goodfiles)
    return results

parser = optparse.OptionParser()
parser.add_option('-c', '--clean', dest='clean', action="store_true", help='Whether to clean before building.', default=False)

(options, args) = parser.parse_args()

os.chdir("..")

if not os.path.exists("build"):
    os.mkdir("build")

# out of source build!
os.chdir("build")

# configure os.system("cmake ..")
os.system("cmake .. -DCODAL_HEADER_EXTRACTION:BOOL=TRUE")

if options.clean:
    os.system("make clean")

# build
#os.system("make -j 10")

#ntpath.basename(f)
folders = [path_leaf(f) for f in glob.glob("../libraries/*/")]
header_folders = [path_leaf(f) for f in glob.glob("./build/*/")]

print folders
print header_folders

mapping = []

#note for next time, need to copy all lib files to their appropriate build/lib place otherwise they get auto cleaned.

valid_libs = []

for folder in header_folders:
    lib_name = "lib" + folder + ".a"
    if not os.path.exists("./"+lib_name):
        print "No library exists, skipping: " + lib_name
        continue

    shutil.copy("./" + lib_name, "./build/"+folder)
    valid_libs = valid_libs + [folder]


for folder in valid_libs:
    lib_name = "lib" + folder + ".a"
    folder_path = '../libraries/' + folder
    header_folder = "./build/" + folder
    header_ext = "includes"

    # get the repo
    try:
        repo = git.Repo('../libraries/' + folder)
    except:
        print folder + " is not a valid git repository."
        continue

    active_branch = repo.active_branch.name

    # check for any uncommitted changes
    if len(repo.index.diff(None)) > 0 :
        print folder + " has uncommitted changes, skipping."
        continue;

    branch_names = [b.name for b in repo.branches]

    # swap to an orphaned branch if none exists
    if "library" not in branch_names:
        repo.active_branch.checkout(orphan="library")

        for folder in glob.glob(folder_path + "/*/"):
            shutil.rmtree(folder)

        files = [f for f in os.listdir('.') if os.path.isfile(f)]

        for file in files:
            os.remove(file)
    else:
        repo.active_branch.checkout("library")

    repo.index.remove("*", r=True)

    copytree(header_folder, folder_path + "/")

    repo.index.add("*")

    author = Actor("codal", "codal@example.com")

    repo.index.commit("Library generated", author=author, committer=author)

    repo.git.checkout(active_branch)

    break
