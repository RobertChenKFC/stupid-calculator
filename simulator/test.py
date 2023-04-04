#!/usr/bin/env python3

from argparse import ArgumentParser
import os
import shutil
import subprocess
import re


"""
Seven segment display:

┌─┐   ╷ ╶─┐ ╶─┐ ╷ ╷ ┌─╴ ┌─╴ ╶─┐ ┌─┐ ┌─┐ 
│ │   │ ┌─┘ ╶─┤ └─┤ └─┐ ├─┐   │ ├─┤ └─┤
└─┘.  ╵.└─╴.╶─┘.  ╵.╶─┘.└─┘.  ╵.└─┘.╶─┘.

"""


def parse_args():
    parser = ArgumentParser(
        description="Run stupid code using node JS."
    )
    parser.add_argument("stu_file", help="Path to stu file.")
    return parser.parse_args()


def modify_code(code):
    code = code.replace("KEY[0]", "readKey()");
    code = code.replace("elif", "else if");
    code = re.sub(
        r"const DISPLAY = \d+;",
        "const DISPLAY = Array(NUM_DISPLAY_DIGITS + 1);",
        code
    )
    return code


def main():
    args = parse_args()
    with open(args.stu_file, "r") as infile:
        code = infile.read()

    code = modify_code(code)

    with open("lib.js", "r") as infile:
        lib_code = infile.read()
    code += lib_code

    js_file = os.path.splitext(os.path.split(args.stu_file)[0])[0] + ".js"
    with open(js_file, "w") as outfile:
        outfile.write(code)

    node = shutil.which("node")
    subprocess.run([node, js_file])


if __name__ == "__main__":
    main()
    
