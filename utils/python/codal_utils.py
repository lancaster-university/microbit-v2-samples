import os
import sys
import platform
import json
import shutil
import re
import subprocess


def system(cmd):
    if os.system(cmd) != 0:
      sys.exit(1)

def build(clean, verbose = False, parallelism = 10):
    # Use Ninja on Windows, or if available in any other OS
    use_ninja = shutil.which("ninja") is not None or platform.system() == "Windows"

    if use_ninja:
        # configure
        system("cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -G \"Ninja\"")

        if clean:
            system("ninja clean")

        # build
        if verbose:
            system("ninja -j {} --verbose".format(parallelism))
        else:
            system("ninja -j {}".format(parallelism))
    else:
        # configure
        system("cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -G \"Unix Makefiles\"")

        if clean:
            system("make clean")

        # build
        if verbose:
            system("make -j {} VERBOSE=1".format(parallelism))
        else:
            system("make -j {}".format(parallelism))

def _strip_json_comments(text):
    result = ""
    i = 0
    n = len(text)
    in_string = False

    while i < n:
        c = text[i]

        if in_string:
            result += c
            if c == '\\' and i + 1 < n:
                result += text[i + 1]
                i += 2
                continue
            if c == '"':
                in_string = False
            i += 1
        elif c == '"':
            in_string = True
            result += c
            i += 1
        elif c == '/' and i + 1 < n and text[i + 1] == '/':
            end = text.find('\n', i)
            if end == -1:
                break
            i = end
        elif c == '/' and i + 1 < n and text[i + 1] == '*':
            end = text.find('*/', i + 2)
            if end == -1:
                break
            # drop the comment but keep newlines so error messages keep sane line numbers
            block = text[i:end + 2]
            result += re.sub(r'[^\n]', '', block)
            i = end + 2
        elif c == ',':
            # tolerate trailing commas, as the CMake parser does
            j = i + 1
            while j < n and text[j] in ' \t\r\n':
                j += 1
            if j < n and text[j] in '}]':
                i = j
            else:
                result += c
                i += 1
        else:
            result += c
            i += 1

    return result

def read_json(fn):
    json_file = ""
    with open(fn) as f:
        json_file = f.read()
    return json.loads(_strip_json_comments(json_file))

def checkgit():
    stat = os.popen('git status --porcelain').read().strip()
    if stat != "":
        print("Missing checkin in", os.getcwd(), "\n" + stat)
        exit(1)

def read_config():
    codal = read_json("codal.json")
    targetdir = codal['target']['name']
    target = read_json("libraries/" + targetdir + "/target.json")
    return (codal, targetdir, target)

def _repo_name(url):
    name = url.rstrip('/').rsplit('/', 1)[-1]
    if name.endswith('.git'):
        name = name[:-4]
    return name

def _is_sha(ref):
    return re.fullmatch(r'[0-9a-fA-F]{7,40}', ref) is not None

def _find_override(branches, name):
    for url, ref in branches.items():
        if _repo_name(url) == name:
            return (url, ref)
    return None

def _git(cwd, args, what=None):
    try:
        subprocess.run(args, cwd=cwd, check=True)
    except subprocess.CalledProcessError:
        if what is None:
            what = "git " + " ".join(args)
        print("{}: failed in {}".format(what, cwd))
        sys.exit(1)

def _default_branch(cwd):
    _git(cwd, ["git", "remote", "set-head", "origin", "-a"])
    return str(subprocess.check_output(
        ["git", "symbolic-ref", "--short", "refs/remotes/origin/HEAD"],
        cwd=cwd), "utf8").strip()

def _ff_only(ref, cwd, name):
    _git(cwd, ["git", "merge", "--ff-only", "origin/" + ref],
         "{}: cannot fast-forward to origin/{} (local branch has diverged)".format(name, ref))

def _git_sync(name, url, ref, cwd, switch=True):
    dirty = str(subprocess.check_output(["git", "status", "--porcelain"], cwd=cwd), "utf8").strip()
    if dirty != "":
        print("{}: refusing to update, uncommitted changes:".format(name))
        print(dirty)
        sys.exit(1)

    _git(cwd, ["git", "remote", "set-url", "origin", url])
    _git(cwd, ["git", "fetch", "origin", "--prune"])

    if ref is None:
        ref = _default_branch(cwd)

    if not switch:
        return

    _git(cwd, ["git", "checkout", ref])
    if not _is_sha(ref):
        _ff_only(ref, cwd, name)

def update(allow_detached=False, sync_dev=False):
    codal = read_json("codal.json")
    targetdir = codal['target']['name']

    target_file = "target-locked.json"
    if codal['target'].get('dev') or sync_dev:
        target_file = "target.json"
    target_path = "libraries/" + targetdir + "/" + target_file
    if not os.path.exists(target_path):
        target_path = "libraries/" + targetdir + "/target.json"
    target = read_json(target_path)

    dirname = os.getcwd()
    branches = codal['target'].get('branches', {})

    for ln in target['libraries']:
        cwd = dirname + "/libraries/" + ln['name']
        override = _find_override(branches, ln['name'])
        if override is not None:
            (url, ref) = override
        else:
            (url, ref) = (ln['url'], ln['branch'])
        if sync_dev:
            ref = None
        _git_sync(ln['name'], url, ref, cwd)

    cwd = dirname + "/libraries/" + targetdir
    _git_sync(targetdir, codal['target']['url'], codal['target']['branch'], cwd,
              switch=not allow_detached)

def revision(rev):
    (codal, targetdir, target) = read_config()
    dirname = os.getcwd()
    os.chdir("libraries/" + targetdir)
    system("git checkout " + rev)
    os.chdir(dirname)
    update(True)

def printstatus( logLines = 3, detail = False ):
    print("\n***%s" % os.getcwd())
    branch = str(subprocess.check_output( [ "git", "branch", "--show-current"] ), "utf8").strip()
    hash   = str(subprocess.check_output( [ "git", "rev-parse", "HEAD" ] ), "utf8").strip()
    tag    = "..."
    try:
        tag = str(subprocess.check_output( [ "git", "describe", "--tags", "--abbrev=0" ], stderr=subprocess.STDOUT ), "utf8").strip()
    except subprocess.CalledProcessError as e:
        tag = "~none~"
    
    print( "Branch: {branch}, Nearest Tag: {tag} ({hash})".format(branch=branch, tag=tag, hash=hash) )
    if detail:
        system( "git --no-pager log -n {} --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit".format(logLines) )
        print( "" )
        
    system("git status -sb")
    print( "" )
    

def status( logLines = 3, detail = True, libs = [] ):
    (codal, targetdir, target) = read_config()
    dirname = os.getcwd()

    if len(libs) == 0:
        for ln in target['libraries']:
            os.chdir(dirname + "/libraries/" + ln['name'])
            printstatus( logLines, detail )
        os.chdir(dirname + "/libraries/" + targetdir)
        printstatus( logLines, detail )
        os.chdir(dirname)
        printstatus( logLines, detail )
    else:
        for lib in libs:
            os.chdir(dirname + "/libraries/" + lib)
            printstatus( logLines, detail )

def get_next_version(options):
    if options.version:
        return options.version
    log = os.popen('git log -n 100').read().strip()
    m = re.search('Snapshot v(\d+)\.(\d+)\.(\d+)(-([\w\-]+).(\d+))?', log)
    if m is None:
        print("Cannot determine next version from git log")
        exit(1)
    v0 = int(m.group(1))
    v1 = int(m.group(2))
    v2 = int(m.group(3))
    vB = -1
    branchName = os.popen('git rev-parse --abbrev-ref HEAD').read().strip()
    if not options.branch and branchName not in ["master","main"]:
        print("On feature branch use -l -b")
        exit(1)
    suff = ""
    if options.branch:
        if m.group(4) and branchName == m.group(5):
            vB = int(m.group(6))
        suff = "-%s.%d" % (branchName, vB + 1)
    elif options.update_major:
        v0 += 1
        v1 = 0
        v2 = 0
    elif options.update_minor:
        v1 += 1
        v2 = 0
    else:
        v2 += 1
    return "v%d.%d.%d%s" % (v0, v1, v2, suff)

def lock(options):
    (codal, targetdir, target) = read_config()
    dirname = os.getcwd()
    for ln in target['libraries']:
        os.chdir(dirname + "/libraries/" + ln['name'])
        checkgit()
        stat = os.popen('git status --porcelain -b').read().strip()
        if "ahead" in stat:
            print("Missing push in", os.getcwd())
            exit(1)
        sha = os.popen('git rev-parse HEAD').read().strip()
        ln['branch'] = sha
        print(ln['name'], sha)
    os.chdir(dirname + "/libraries/" + targetdir)
    ver = get_next_version(options)
    print("Creating snaphot", ver)
    system("git checkout target-locked.json")
    checkgit()
    target["snapshot_version"] = ver
    with open("target-locked.json", "w") as f:
        f.write(json.dumps(target, indent=4, sort_keys=True))
    system("git commit -am \"Snapshot %s\"" % ver)  # must match get_next_version() regex
    sha = os.popen('git rev-parse HEAD').read().strip()
    system("git tag %s" % ver)
    system("git pull")
    system("git push")
    system("git push --tags")
    os.chdir(dirname)
    print("\nNew snapshot: %s [%s]" % (ver, sha))

def delete_build_folder(in_folder = True):
    if in_folder:
        os.chdir("..")

    shutil.rmtree('./build')
    os.mkdir("./build")

    if in_folder:
        os.chdir("./build")

def generate_docs():
    from doc_gen.doxygen_extractor import DoxygenExtractor
    from doc_gen.md_converter import MarkdownConverter
    from doc_gen.system_utils import SystemUtils
    from doc_gen.doc_gen import generate_mkdocs

    os.chdir("..")
    (codal, targetdir, target) = read_config()

    lib_dir = os.getcwd() + "/libraries/"

    libraries = [lib_dir + targetdir]

    for l in target["libraries"]:
        libraries = libraries + [ lib_dir + l["name"]]

    os.chdir(lib_dir + targetdir)

    generate_mkdocs(libraries)


