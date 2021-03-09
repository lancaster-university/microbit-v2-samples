"""
Automatic flashing utility for the micro:bit.

The function find_microbit comes from project #'uFlash':
Copyright (c) 2015-2020 Nicholas H.Tollervey and others.
MIT License https://github.com/ntoll/uflash/blob/master/LICENSE

The rest of the code here has the following copyright and license:

The MIT License (MIT)

Copyright (c) 2021 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"""

from __future__ import print_function

import ctypes
import os
import sys
from subprocess import check_output

def find_microbit():
    """
    Returns a path on the filesystem that represents the plugged in BBC
    micro:bit that is to be flashed. If no micro:bit is found, it returns
    None.
    Works on Linux, OSX and Windows. Will raise a NotImplementedError
    exception if run on any other operating system.
    """
    # Check what sort of operating system we're on.
    if os.name == 'posix':
        # 'posix' means we're on Linux or OSX (Mac).
        # Call the unix "mount" command to list the mounted volumes.
        mount_output = check_output('mount').splitlines()
        mounted_volumes = [x.split()[2] for x in mount_output]
        for volume in mounted_volumes:
            if volume.endswith(b'MICROBIT'):
                return volume.decode('utf-8')  # Return a string not bytes.
    elif os.name == 'nt':
        # 'nt' means we're on Windows.

        def get_volume_name(disk_name):
            """
            Each disk or external device connected to windows has an attribute
            called "volume name". This function returns the volume name for
            the given disk/device.
            Code from http://stackoverflow.com/a/12056414
            """
            vol_name_buf = ctypes.create_unicode_buffer(1024)
            ctypes.windll.kernel32.GetVolumeInformationW(
                ctypes.c_wchar_p(disk_name), vol_name_buf,
                ctypes.sizeof(vol_name_buf), None, None, None, None, 0)
            return vol_name_buf.value

        #
        # In certain circumstances, volumes are allocated to USB
        # storage devices which cause a Windows popup to raise if their
        # volume contains no media. Wrapping the check in SetErrorMode
        # with SEM_FAILCRITICALERRORS (1) prevents this popup.
        #
        old_mode = ctypes.windll.kernel32.SetErrorMode(1)
        try:
            for disk in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
                path = '{}:\\'.format(disk)
                #
                # Don't bother looking if the drive isn't removable
                #
                if ctypes.windll.kernel32.GetDriveTypeW(path) != 2:
                    continue
                if os.path.exists(path) and \
                        get_volume_name(path) == 'MICROBIT':
                    return path
        finally:
            ctypes.windll.kernel32.SetErrorMode(old_mode)
    else:
        # No support for unknown operating systems.
        raise NotImplementedError('OS "{}" not supported.'.format(os.name))

def flash(hx_path):
    print("Flashing to micro:bit...")

    mb_path = find_microbit()

    if mb_path:
        new_hx_path = os.path.join(mb_path, "MICROBIT.hex")

        print("micro:bit path: " + mb_path)
        print("hex path: " + hx_path)

        try:
            with open(hx_path, "r") as hex_read:
                user_hex = hex_read.read()
        except:
            sys.exit("There was a problem reading the .hex file; did you forget to build?")

        try:
            with open(new_hx_path, "wb") as output:
                output.write(user_hex.encode('ascii'))
                output.flush()
                os.fsync(output.fileno())
        except:
            sys.exit("There was a problem writing to the micro:bit.")

        print("Hex flashed successfully.")

    else:
        sys.exit("Could not find a connected micro:bit!")
