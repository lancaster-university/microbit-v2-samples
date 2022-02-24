#!/usr/bin/env python3

# The MIT License (MIT)

# Copyright (c) 2022 Lancaster University.

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

import json
import os
import shutil
import sys
from syslog import LOG_WARNING
import urllib.request
import optparse
import subprocess

from colorama import Fore, Back, Style
from importlib import import_module
from genericpath import exists

TOOLCHAIN_URL = "https://github.com/lancaster-university/codal.git"
TARGET_LIST = [
    "https://raw.githubusercontent.com/lancaster-university/codal/master/utils/targets.json"
]
BASE_ROOT = os.getcwd()

def log_info( text ):
  print( Style.BRIGHT + text + Style.RESET_ALL )

def log_warn( text ):
  print( Style.BRIGHT + Fore.YELLOW + text + Style.RESET_ALL )

def log_error( text ):
  print( Style.BRIGHT + Fore.RED + text + Style.RESET_ALL )

log_warn( "WARNING: This is a CODAL-Bootstrap build. CODAL-Bootstrap is still in ALPHA, expect bugs!" )

class PassThroughOptionParser(optparse.OptionParser):
  def _process_args(self, largs, rargs, values):
    while rargs:
      try:
        optparse.OptionParser._process_args(self, largs, rargs,values)
      except (optparse.BadOptionError, optparse.AmbiguousOptionError) as e:
        largs.append(e.opt_str)

def create_tree():
  path_list = [
    "libraries",
    "build",
    "source"
  ]
  for p in path_list:
    if not exists( os.path.join( BASE_ROOT, p ) ):
      os.mkdir( os.path.join( BASE_ROOT, p ) )
  
  with open(".gitignore", 'w') as git_ignore:
    git_ignore.writelines( [
      ".vscode\n",
      ".yotta.json\n",
      "*.bin\n",
      "*.DS_Store\n",
      "*.hex\n",
      "*.pyc\n",
      "*.swp\n",
      "*.uf2\n",
      "*.pyc\n",
      "*~\n",
      "build\n",
      "buildcache.json\n",
      "codal.json\n",
      "libraries\n",
      "Makefile\n",
      "pxtapp\n",
      "yotta_modules\n",
      "yotta_targets\n"
    ] )

def download_targets():
  log_info( "Downloading valid targets..." )
  cache = {}
  for url in TARGET_LIST:
    r = urllib.request.urlopen( url )
    for t in json.load( r ):
      cache[ t["name"] ] = t
  return cache

def library_clone( url, name, branch = "master", specfile = "module.json" ):
  log_info( f'Downloading library {name}...' )
  git_root = os.path.join( BASE_ROOT, 'libraries', name )
  if not exists( os.path.join( git_root, '.git' ) ):
    os.system( f'git clone --recurse-submodules --branch "{branch}" "{url}" "{git_root}"' )

  if exists( os.path.join( git_root, specfile ) ):
    return load_json( os.path.join( git_root, specfile ) )

  log_warn( f'WARN: Missing specification file for {name}: {specfile}' )
  return {}

def library_update( name, branch="", specfile = "module.json"):
  log_info( f'Updating library {name}...' )
  git_root = os.path.join( BASE_ROOT, 'libraries', name )
  if not exists( git_root ):
    raise Exception( f'No such library {name}' )
  
  if branch != "":
    try:
      subprocess.run( f'git checkout {branch}', cwd=git_root, shell=True )
    except subprocess.CalledProcessError as err:
      raise Exception( f'No such branch {branch} for library {name}' )

  try:
    subprocess.run( "git pull", cwd=git_root, shell=True )
  except subprocess.CalledProcessError as err:
    raise Exception( 'Unable to pull changes for ${name}' )

  if exists( os.path.join( git_root, specfile ) ):
    return load_json( os.path.join( git_root, specfile ) )

  log_warn( f'WARN: Missing specification file for {name}: {specfile}' )
  return {}

def library_version( name ):
  try:
    return subprocess.check_output( "git rev-parse --short HEAD", cwd=os.path.join( BASE_ROOT, "libraries", name ), shell=True ).decode( 'utf-8' )
  except subprocess.CalledProcessError as err:
    print( "library_version error:", err )
    return ""
  except FileNotFoundError as err:
    return ""

def load_json( path ):
  with open(path, 'r') as src:
    return json.load( src )

def go_configure( info, config={} ):
  create_tree()
  library_clone( TOOLCHAIN_URL, "codal", branch="feature/bootstrap" )

  # Copy out the base CMakeLists.txt, can't run from the library, and this is a CMake limitation
  # Note; use copy2 here to preserve metadata
  shutil.copy2(
    os.path.join( BASE_ROOT, "libraries", "codal", "CMakeLists.txt" ),
    os.path.join( BASE_ROOT, "CMakeLists.txt" )
  )

  log_info( "Downloading target support files..." )
  details = library_clone( info["url"], info["name"], branch = info["branch"], specfile = "target.json" )

  # This is _somewhat_ redundant as cmake does this as well, but it might be worth doing anyway as there might be
  # additional library files needed for other, as-yet unidentified features. Plus, it makes the build faster afterwards -JV
  log_info( "Downloading libraries..." )
  for lib in details["libraries"]:
    library_clone( lib["url"], lib["name"], branch = lib["branch"] )

  with open( os.path.join( BASE_ROOT, "codal.json" ), "w" ) as codal_json:
    config["target"] = info
    config["target"]["test_ignore"] = True
    config["target"]["dev"] = True

    json.dump( config, codal_json, indent=4 )
  
  print( "\n" )
  print( Style.BRIGHT + "All done! You can now start developing your code in the source/ folder. Running ./build.py will now defer to the actual build tools" )
  print( "Happy coding!" + Style.RESET_ALL )
  print( "" )

def list_valid_targets():
  targets = download_targets()
  for t in targets:
    print( f'{t:<30}: {targets[t]["info"]}' )

if exists( os.path.join(BASE_ROOT, "codal.json") ) and exists( os.path.join(BASE_ROOT, "libraries", "codal", "build.py") ):
  parser = PassThroughOptionParser(add_help_option=False)
  parser.add_option('--bootstrap', dest='force_bootstrap', action="store_true", default=False)
  (options, args) = parser.parse_args()

  if options.force_bootstrap:
    log_warn( "WARNING: '--bootstrap' forces bootstrap to take over, downloaded build tools will not be run!" )

  if not options.force_bootstrap:
    sys.path.append( os.path.join(BASE_ROOT, "libraries", "codal") )
    import_module( f'libraries.codal.build' )
    exit(0)

parser = optparse.OptionParser(usage="usage: %prog target-name [options]", description="BOOTSTRAP MODE - Configures the current project directory for a specified target. Will defer to the latest build tools once configured.")
parser.add_option('--bootstrap', dest='force_bootstrap', action='store_true', help="Skips any already downloaded build toolchain, and runs in bootstrap mode directly.", default=False)
parser.add_option('--ignore-codal', dest='ignore_codal', action='store_true', help="Skips any pre-existing codal.json in the project folder, and runs as if none exists.", default=False)
parser.add_option('--merge-upstream-target', dest='merge_upstream_target', action='store_true', help="Keeps the existing codal.json, but only for non-target parameters, merging the new target definition in with the old arguments.", default=False)
parser.add_option('-u', dest='update', action='store_true', help="Update this file and the build tools library", default=False)
(options, args) = parser.parse_args()

if options.update:
  log_info( "Attempting to automatically update bootstrap..." )
  old_vers = library_version( 'codal-bootstrap' )
  if exists(os.path.join( BASE_ROOT, "libraries", "codal-bootstrap" )):
    library_update( "codal-bootstrap" )
  else:
    library_clone( "https://github.com/lancaster-university/codal-bootstrap.git", "codal-bootstrap", branch="main" )
  vers = library_version( 'codal-bootstrap' )
  
  if vers == old_vers:
    log_info( "Nothing to update, codal-bootstrap is already the latest version" )
    exit( 0 )

  log_info( "Downloaded a new version of bootstrap, updating the project files..." )
  shutil.copy2(
    os.path.join( BASE_ROOT, "libraries", "codal-bootstrap", "build.py" ),
    os.path.join( BASE_ROOT, "build.py" )
  )

  log_info( "Done! Happy coding :)\n" )

  exit(0)

if len(args) == 0:
  # We might have an existing device config already, so grab that and try and pull that...
  if exists( os.path.join( BASE_ROOT, "codal.json" ) ) and not options.ignore_codal:
    log_info( "Project already has a codal.json, trying to use that to determine the build system and any missing dependencies..." )
    local_config = load_json( os.path.join( BASE_ROOT, "codal.json" ) )
    local_target = local_config["target"]

    print( "Configuring from codal.json!" )
    go_configure( local_target )
    exit(0)

  log_warn( "Please supply an initial target to build against:" )
  list_valid_targets()
  exit( 0 )

if len(args) == 1:

  # 'Magic' target to list all targets
  if args[0] == "ls":
    log_info( "Available target platforms:" )
    list_valid_targets()
    exit( 0 )
  
  targets = download_targets()
  query = args[0]

  if query not in targets:
    log_error( "Invalid or unknown target, try './build.py ls' to see available targets" )
    exit( 1 )

  local_config = {}
  if options.merge_upstream_target:
    log_info( "Preserving local configuration, but ignoring the target and using supplied user target..." )
    local_config = load_json( os.path.join( BASE_ROOT, "codal.json" ) )

  go_configure( targets[query], config=local_config )