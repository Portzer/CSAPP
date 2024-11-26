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
        "isa" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function", "-Wno-unused-variable",
                "-I", "./src",
                "-DDEBUG_INSTRUCTION_CYCLE",
                "./src/common/convert.c",
                "./src/common/cleanup.c",
                "./src/algorithm/hashtable.c",
                "./src/algorithm/trie.c",
                "./src/algorithm/array.c",
                "./src/hardware/cpu/isa.c",
                "./src/hardware/cpu/mmu.c",
                "./src/hardware/memory/dram.c",
                "./src/tests/test_run_isa.c",
                "-o", "./bin/run_isa"
            ]
        ],
        "int" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_INSTRUCTION_CYCLE",
                # "-DDEBUG_ENABLE_SRAM_CACHE",
                "-DUSE_NAVIE_VA2PA",
                "./src/common/convert.c",
                "./src/common/cleanup.c",
                "./src/algorithm/hashtable.c",
                "./src/algorithm/trie.c",
                "./src/algorithm/array.c",
                "./src/hardware/cpu/isa.c",
                "./src/hardware/cpu/mmu.c",
                "./src/hardware/cpu/inst.c",
                # "./src/hardware/cpu/sram.c",
                "./src/hardware/cpu/interrupt.c",
                "./src/hardware/memory/dram.c",
                # "./src/hardware/memory/swap.c",
                "./src/process/syscall.c",
                "./src/tests/test_run_isa.c",
                "-o", "./bin/run_isa"
            ]
        ],
        "inst" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function", "-Wno-unused-variable",
                "-I", "./src",
                "-DDEBUG_INSTRUCTION_CYCLE",
                "./src/common/convert.c",
                "./src/common/cleanup.c",
                "./src/algorithm/hashtable.c",
                "./src/algorithm/trie.c",
                "./src/algorithm/array.c",
                "./src/hardware/cpu/inst.c",
                "./src/tests/test_inst.c",
                "-o", "./bin/test_inst"
            ],
        ],
        "link" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
                "-I", "./src",
                "-shared", "-fPIC",
                "./src/common/convert.c",
                "./src/common/cleanup.c",
                "./src/algorithm/array.c",
                "./src/algorithm/hashtable.c",
                "./src/algorithm/linkedlist.c",
                "./src/linker/parseElf.c",
                "./src/linker/staticlink.c",
                "-o", "./bin/staticLinker.so"
            ],
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
                "-I", "./src",
                # "-DDEBUG_LINK",
                "./src/common/convert.c",
                "./src/common/cleanup.c",
                "./src/algorithm/array.c",
                "./src/algorithm/hashtable.c",
                "./src/algorithm/linkedlist.c",
                "./src/linker/linker.c",
                "-ldl", "-o", "./bin/link"
            ],
        ],
        "elf" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99",
                "-Wno-unused-function",
                "-Wno-unused-but-set-variable",
                "-I", "./src",
                "-DDEBUG_PARSE_ELF",
                "-DDEBUG_LINK",
                "./src/common/convert.c",
                "./src/common/cleanup.c",
                "./src/algorithm/array.c",
                "./src/algorithm/hashtable.c",
                "./src/algorithm/linkedlist.c",
                "./src/linker/parseElf.c",
                "-o", "./bin/elf"
            ],
        ],
        "mesi" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable",
                "-I", "./src",
                # "-DDEBUG",
                "./src/mains/mesi.c",
                "-o", "./bin/mesi"
            ],
        ],
        "false_sharing" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable",
                "-I", "./src",
                "-pthread",
                "./src/mains/false_sharing.c",
                "-o", "./bin/false_sharing"
            ],
        ],
        "rbt" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_REDBLACK",
                "./src/common/convert.c",
                "./src/algorithm/bst.c",
                "./src/algorithm/rbt.c",
                "./src/tests/test_rbt.c",
                "-o", "./bin/rbt"
            ],
        ],
        "trie" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_TRIE",
                "./src/algorithm/trie.c", "./src/algorithm/hashtable.c",
                "./src/tests/test_trie.c",
                "-o", "./bin/trie"
            ],
        ],
        "bst" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_BST",
                "./src/algorithm/bst.c",
                "./src/common/convert.c",
                "./src/tests/test_bst.c",
                "-o", "./bin/bst"
            ],
        ],
        "malloc" : [
            [
                "/usr/bin/",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_MALLOC",
                # "-DIMPLICIT_FREE_LIST",
                # "-DEXPLICIT_FREE_LIST",
                "-DREDBLACK_TREE",
                "./src/common/convert.c",
                "./src/algorithm/linkedlist.c",
                "./src/algorithm/bst.c",
                "./src/algorithm/rbt.c",
                "./src/malloc/block.c",
                "./src/malloc/small_list.c",
                "./src/malloc/implicit_list.c",
                "./src/malloc/explicit_list.c",
                "./src/malloc/segregated_list.c",
                "./src/malloc/redblack_tree.c",
                "./src/malloc/mem_alloc.c",
                "./src/tests/test_malloc.c",
                "-o", "./bin/malloc"
            ],
        ],
        "convert" : [
            [
                "/usr/bin/gcc",
                "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                "-I", "./src",
                "-DDEBUG_BST",
                "-DDEBUG_STRING2UINT",
                "./src/common/convert.c",
                "./src/tests/test_convert.c",
                "-o", "./bin/convert"
            ],
        ],
    }

    if not key in gcc_map:
        print("input the correct build key:", gcc_map.keys())
        exit()
    for command in gcc_map[key]:
        subprocess.run(command)

def run(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        "inst" : ["./bin/test_inst"],
        "isa" : ["./bin/run_isa"],
        "int" : ["./bin/run_isa"],
        "elf" : ["./bin/elf"],
        "link" : ["./bin/link", "main", "sum", "-o", "output"],
        "mesi" : ["./bin/mesi"],
        "false_sharing" : ["./bin/false_sharing"],
        "rbt" : ["./bin/rbt"],
        "trie" : ["./bin/trie"],
        "bst" : ["./bin/bst"],
        "malloc" : ["./bin/malloc"],
        "convert" : ["./bin/convert"],
    }
    if not key in bin_map:
        print("input the correct binary key:", bin_map.keys())
        exit()
    subprocess.run(bin_map[key])

def debug(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        "isa" : [gdb, "./bin/run_isa"],
        "int" : [gdb, "./bin/run_isa"],
        "link" : [gdb, "--args", "./bin/link", "main", "sum", "-o", "output"],
        "malloc" : [gdb, "./bin/malloc"],
        "bst" : [gdb, "./bin/bst"],
        "rbt" : [gdb, "./bin/rbt"],
        "trie" : [gdb, "./bin/trie"],
        "inst" : [gdb, "./bin/test_inst"],
    }
    if not key in bin_map:
        print("input the correct binary key:", bin_map.keys())
        exit()
    subprocess.run(["/usr/bin/gdb", bin_map[key]])

def mem_check(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        "isa" : ["./bin/ut_isa"],
        "link" : ["./bin/link", "main", "sum", "-o", "output"]
    }
    if not key in bin_map:
        print("input the correct memory check key:", bin_map.keys())
        exit()
    subprocess.run([
                       "/usr/bin/valgrind",
                       "--tool=memcheck",
                       "--leak-check=full"] + bin_map[key])

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
elif operation == "memcheck":
    assert(len(sys.argv) == 3)
    mem_check(sys.argv[2])
elif operation == "count":
    count_lines()
elif operation == "clean":
    pass
elif operation == "csim":
    cache_verify()