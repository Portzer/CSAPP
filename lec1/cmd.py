#!/usr/bin/python3
# usage:
#   ./cmd.py argv[1] argv[2], ...
#   /usr/bin/python3 ./cmd.py argv[1] argv[2], ...
import sys
import os
import subprocess
from pathlib import Path

# print arguments
i = 0
for argv in sys.argv:
    print("[", i, "] ", argv)
    i += 1

KEY_MACHINE = "m"
KEY_LINKER = "l"

EXE_BIN_LINKER = "./bin/test_elf"
EXE_BIN_MACHINE = "./bin/test_machine"



def make_build_directory():
    if not os.path.isdir("./bin/"):
        os.mkdir("./bin/")

def format_include(s):
    a = "#include<headers/"
    b = "#include<"

    # check include
    if s.startswith(a):
        s = "#include \"headers/" + s[len(a):]
        for j in range(len(s)):
            if s[j] == '>':
                l = list(s)
                l[j] = "\""
                s = "".join(l)
    elif s.startswith(b):
        s = "#include <" + s[len(b):]
    return s

def format_whiteline(s):
    space = 0
    for c in s:
        if c == ' ':
            space += 1
    if space == len(s) - 1 and s[-1] == '\n':
        s = "\n"
    return s
def count_lines():
    # get files with paths
    filelist = list(Path(".").rglob("*.[ch]"))
    name_count = []
    total_count = 0
    maxfilename = 0
    for filename in filelist:
        count = 0
        for index, line in enumerate(open(filename, 'r')):
            count += 1
        name_count += [[str(filename), count]]
        total_count += count
        if len(str(filename)) > maxfilename:
            maxfilename = len(str(filename))
    # print result
    print("count .c and .h file lines:")
    sortedlist = sorted(name_count, key = lambda x: x[1], reverse=True)
    for [filename, count] in sortedlist:
        print(filename, end="")
        n = (int(maxfilename / 4) + 1) * 4
        for i in range(n - len(filename)):
            print(" ", end="")
        print(count)
    print("\nTotal:", total_count)

def build(key):
    make_build_directory()
    gcc_map = {
        KEY_MACHINE : [
            "/usr/bin/gcc",
            "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
            "-I", "./src",
            "./src/test/test_machine.c",
            "./src/common/print.c",
            "./src/common/convert.c",
            "./src/common/cleanup.c",
            "./src/datastruct/trie.c",
            "./src/datastruct/array.c",
            "./src/hardware/cpu/isa.c",
            "./src/hardware/cpu/mmu.c",
            "./src/hardware/memory/dram.c",
            "-o", EXE_BIN_MACHINE],
        KEY_LINKER : [
            "/usr/bin/gcc",
            "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
            "-I", "./src",
            "./src/test/test_elf.c",
            "./src/common/print.c",
            "./src/common/tagmalloc.c",
            "./src/common/cleanup.c",
            "./src/datastruct/array.c",
            "./src/common/convert.c",
            "./src/datastruct/hashtable.c",
            "./src/datastruct/linkedlist.c",
            "./src/linker/parseElf.c",
            "./src/linker/static_link.c",
            "-o", EXE_BIN_LINKER
        ],
        "mesi" : [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
                "-I", "./src",
              #  "-DDEBUG",
                "./src/hardware/cpu/mesi.c",
                "-o", "./bin/mesi"
        ],
        "malloc" : [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_MALLOC",
                #"-DIMPLICIT_FREE_LIST",
                 "-DEXPLICIT_FREE_LIST",
                # "-DFREE_BINARY_TREE",
                "./src/test/mem_alloc.c",
                "-o", "./bin/malloc"
        ],
    }
    if not key in gcc_map:
        print("input the correct build key:", gcc_map.keys())
        exit()
    subprocess.run(gcc_map[key])

def run(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        KEY_MACHINE : EXE_BIN_MACHINE,
        KEY_LINKER : EXE_BIN_LINKER,
        "mesi" : "./bin/mesi",
        "malloc" : "./bin/malloc"
    }
    if not key in bin_map:
        print("input the correct binary key:", bin_map.keys())
        exit()
    subprocess.run([bin_map[key]])

def debug(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        KEY_MACHINE : EXE_BIN_MACHINE,
        KEY_LINKER : EXE_BIN_LINKER
    }
    if not key in bin_map:
        print("input the correct binary key:", bin_map.keys())
        exit()
    subprocess.run(["/usr/bin/gdb", bin_map[key]])

def mem_check(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        KEY_MACHINE : EXE_BIN_MACHINE,
        KEY_LINKER : EXE_BIN_LINKER
    }
    if not key in bin_map:
        print("input the correct memory check key:", bin_map.keys())
        exit()
    subprocess.run([
        "/usr/bin/valgrind",
        "--tool=memcheck",
        "--leak-check=full",
        bin_map[key]
    ])

def cache_verify():
    subprocess.run([
        "/usr/bin/python3",
        "./src/hardware/cpu/cache_verify.py",
        "/mnt/e/Ubuntu/cache/csim-ref",
        "/mnt/e/Ubuntu/cache/traces/"
    ])

# main
assert(len(sys.argv) >= 2)
operation = sys.argv[1].lower()

if operation == "build":
    assert(len(sys.argv) == 3)
    build(sys.argv[2])
elif operation == "run":
    assert(len(sys.argv) == 3)
    run(sys.argv[2])
elif operation == "debug":
    assert(len(sys.argv) == 3)
    debug(sys.argv[2])
elif operation.lower() == KEY_MACHINE.lower():
    build(KEY_MACHINE)
    run(KEY_MACHINE)
elif operation.lower() == KEY_LINKER.lower():
    build(KEY_LINKER)
    run(KEY_LINKER)
elif operation == "memorycheck":
    assert(len(sys.argv) == 3)
    mem_check(sys.argv[2])
elif operation == "count":
    count_lines()
elif operation == "clean":
    pass
elif operation == "csim":
    cache_verify()