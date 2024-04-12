# Copyright 2023 Sony Semiconductor Solutions Corp. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import struct
import sys
import os

# index of list in size_check_table
SECTION_NAME = 0
CHECK_FLAG = 1
MAX_SECTION_SIZE = 2

# aot file size
MAX_AOT_SIZE = 1572864  # 1.5MB = 1572864
SIGNATURE_SIZE = 160
MAX_DEPLOYABLE_SIZE = MAX_AOT_SIZE - SIGNATURE_SIZE

# max section size
MAX_TEXT_SECTION_SIZE = 1376256  # 1.3125MB = 1376256

# section line
SECTION_LINE = (
    "--------------------------------------------------------------------------------"
)

size_check_table = [
    # [section name, size check flag, max section size (byte)]
    ["Target Info", False, 0],
    ["Init Data", False, 0],
    ["Text", True, MAX_TEXT_SECTION_SIZE],
    ["Function", False, 0],
    ["Export", False, 0],
    ["Relocation", False, 0],
    ["Custom", False, 0],
]

size_list = [
    SECTION_LINE,
    "[Size per Section of AOT File]",
    "section name  size(byte)  limit(byte)",
    "--------------------------------------",
]


def main():
    if len(sys.argv) < 2:
        print("Usage: check_aot_size.py [aot_filename]")
        return

    aot_file_name = sys.argv[1]

    # Get aot size and section size
    aot_file_size = get_aot_size(aot_file_name)
    section_size_dict = parse_aot(aot_file_name)

    # Check size Result
    check_size(aot_file_size, section_size_dict)

    # show size per section
    show_size_list()
    print(SECTION_LINE)


def get_aot_size(aot_file_name):
    aot_size = os.path.getsize(aot_file_name)
    return aot_size


def parse_aot(aot_file_name):
    section_size_dict = {}

    with open(aot_file_name, "rb") as f:
        # Magic Number
        b = f.read(4)
        if b != b"\x00aot":
            sys.exit(1)

        # Version
        b = f.read(4)

        section, size = struct.unpack("<ii", f.read(4 * 2))

        while size:
            if section > len(size_check_table):
                sys.exit(0)

            section_name = size_check_table[section][SECTION_NAME]

            if size_check_table[section][CHECK_FLAG]:
                limit = size_check_table[section][MAX_SECTION_SIZE]

            else:
                limit = "unlimited"

            size_list.append(
                f"{section_name:13} {str(size).rjust(10)}  {str(limit).rjust(11)}"
            )

            section_size_dict[section_name] = size

            if size % 4 != 0:
                size = size + 4 - size % 4

            f.seek(size, 1)

            try:
                section, size = struct.unpack("<ii", f.read(4 * 2))
            except struct.error:
                break

    return section_size_dict


def check_size(aot_file_size, section_size_dict):
    # Check aot file size
    print(SECTION_LINE)
    print("[AOT File Size Check]")

    if aot_file_size >= MAX_DEPLOYABLE_SIZE:
        print("Result: Failure")
        print("Your AOT file size is too large to deploy.")
    else:
        print("Result: Success")
    print(" Your AOT file size :", str(aot_file_size).rjust(8), "byte")
    print(" Max AOT file size  :", str(MAX_DEPLOYABLE_SIZE).rjust(8), "byte")

    # Check section size
    print(SECTION_LINE)

    for section_table in size_check_table:
        if section_table[CHECK_FLAG]:
            section_name = section_table[SECTION_NAME]
            max_section_size = section_table[MAX_SECTION_SIZE]
            if section_name in section_size_dict.keys():
                section_size = section_size_dict[section_name]
                print(f"[{section_name} Section Size of AOT File Check]")
                if section_size >= max_section_size:
                    print("Result: Failure")
                    print(
                        f"Your AOT {section_name} section size is too large to deploy."
                    )
                else:
                    print("Result: Success")

                print(
                    f" {f'Your AOT {section_name} section size':26}",
                    ":",
                    str(section_size).rjust(8),
                    "byte",
                )
                print(
                    f" {f'Max AOT {section_name} section size':26}",
                    ":",
                    str(max_section_size).rjust(8),
                    "byte",
                )


def show_size_list():
    # print size per section
    for str in size_list:
        print(str)


if __name__ == "__main__":
    main()
