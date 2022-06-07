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

import os
import sys

from importlib import import_module
from genericpath import exists

BOOTSTRAP_TAG = "main"
TARGET_LIST = [
    "https://raw.githubusercontent.com/lancaster-university/codal/master/utils/targets.json"
]
BASE_ROOT = os.getcwd()
BOOTSTRAP_ROOT = os.path.join( BASE_ROOT, 'libraries', 'codal-bootstrap' )

# Minimum folder structure:
if not exists( os.path.join( BASE_ROOT, 'libraries' ) ):
  os.mkdir( os.path.join( BASE_ROOT, 'libraries' ) );

# Grab the latest library
if not exists( os.path.join( BASE_ROOT, 'libraries', 'codal-bootstrap' ) ):
  print( "Downloading codal-bootstrap..." )
  if not exists( os.path.join( BOOTSTRAP_ROOT, '.git' ) ):
    os.system( f'git clone --recurse-submodules --branch "{BOOTSTRAP_TAG}" "https://github.com/lancaster-university/codal-bootstrap.git" "{BOOTSTRAP_ROOT}"' )

# Jump into the current upstream code
sys.path.append( BOOTSTRAP_ROOT )
bootstrap = import_module( f'libraries.codal-bootstrap.bootstrap' )
bootstrap.go_bootstrap( TARGET_LIST )
