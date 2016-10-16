'''
    WSkS Test Bench

    @author: Tomas Fiedor, ifiedortom@fit.vutbr.cz
    @summary: Test Bench script for running several benchmarks on binaries

'''

import argparse
from datetime import datetime
import itertools
import os
import re
import subprocess
import sys
from threading import Timer
from termcolor import colored


def createArgumentParser():
    '''
    Creates Argument Parser object
    '''
    parser = argparse.ArgumentParser("WSkS Test Gen")
    parser.add_argument('--dir', '-d', default=(os.path.join(os.curdir, "benchmarks")), help="directory with benchmarks")
    parser.add_argument('--generate', '-g', default=None, nargs=2, help='generates parametrized benchmark up to N')
    parser.add_argument('--generate-alt', '-a', default=None, nargs=3, help='generates parametrized benchmarks up to N with ALT alternation')
    return parser

def parse_arguments():
    '''
    Parse input arguments
    '''
    parser = createArgumentParser()
    if len(sys.argv) == 0:
        parser.print_help()
        quit()
    else:
        return parser.parse_args()

# methods for generating


def generate_horn_sub(n):
    '''
    Generate simple horn formula in form of:

    ws1s;
    ex2 X: all2 X1...Xn: & (Xi sub X => Xi+1 sub X)

    @param n: parameter n
    '''
    if n < 2:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n" + "ex2 X: all2 "
    string += ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": "
    string += " & ".join(["(X{0} sub X => X{1} sub X)".format(i, i+1) for i in range(1, n)]) + ";"
    return string


def generate_simple(n):
    '''
    '''
    if n < 2:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n"
    string += "ex2 " + ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": "
    string += " & ".join(["(X{0} sub X{1})".format(i, i+1) for i in range(1, n)]) + ";"
    return string


def generate_horn_sub_alt(n):
    '''
    Generate simple horn formula in form of:

    ws1s;
    ex2 X: all2 X1...Xn: & (Xi sub X => Xi+1 sub X)

    @param n: parameter n
    '''
    if n < 2:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n" + "ex2 X: all2 "
    string += ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": "
    string += " & ".join(["( (X{0} sub X & X{0} ~= X{1}) => X{1} sub X)".format(i, i+1) for i in range(1, n)]) + ";"
    return string


def generate_set_singletons(n):
    '''
    Generate simple horn formula in form of:

    ws1s;
    ex2 X: all2 X1...Xn: & (Xi sub X => Xi+1 sub X)

    @param n: parameter n
    '''
    if n < 1:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n"
    string += "ex2 " + ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": "
    string += "all1 x1, x2: "
    string += " & ".join(["((x1 in X{0} & x2 in X{0}) => x1 = x2)".format(i) for i in range(1, n+1)]) + ";"
    return string


def generate_set_obvious(n):
    '''
    Generate simple horn formula in form of:

    ws1s;
    ex2 X: all2 X1...Xn: & (Xi sub X => Xi+1 sub X)

    @param n: parameter n
    '''
    if n < 1:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n"
    string += "ex2 " + ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": "
    string += "all2 X: ("
    string += " & ".join(["((X sub X{0} & X ~= X{0}) => ~X{0} sub X)".format(i) for i in range(1, n+1)]) + ");"
    return string


def generate_set_closed(n):
    '''
    Generate simple horn formula in form of:

    ws1s;
    ex2 X: all2 X1...Xn: & (Xi sub X => Xi+1 sub X)

    @param n: parameter n
    '''
    if n < 1:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n"
    string += "ex2 " + ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": "
    string += "all1 x: ex1 y, z: ~("
    string += " & ".join(["( (x in X{0} & x <= y & y <= z & z in X{0}) => y in X{0} )".format(i) for i in range(1, n+1)]) + ");"
    return string


def generate_horn_trans(n):
    '''

    '''
    if n < 3:
        print("[*] Skipping n = {}".format(n))
        return None

    all_combinations = list(itertools.permutations(range(1, n+1), 3))

    string = "ws1s;\n" + "all2 X: ex2 "
    string += ", ".join(["X" + str(i) for i in range(1, n+1)]) + ": ~("
    string += " & ".join(["( (X{0} sub X{1} & X{1} sub X{2}) => X{0} sub X{2})".format(a, b, c) for (a, b, c) in all_combinations])
    string += ") & "
    string += " & ".join(["X{0} sub X".format(i) for i in range(1, n+1)]) + ";"
    return string


def generate_toss(n):
    if n < 2:
        print("[*] Skipping n = {}".format(n))
        return None

    string = "ws1s;\n" + "ex2 X: all1 "
    string += ", ".join(["x" + str(i) for i in range(1, n+1)]) + ": ("
    string += " & ".join(["(x{0} in X => x{1} in X)".format(i, i+1) for i in range(1, n)])
    string += ");"
    return string


def generate_horn_sub_odd_alts(n, alt):
    if n < alt+1:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n" + "ex2 X: "
    for i in range(1, alt+1):
        if i % 2 == 0:
            string += "ex2"
        else:
            string += "all2"
        string += " X{}: ".format(i) if i != alt else " "
    string += ", ".join(["X" + str(i) for i in range(alt, n+1)]) + ": "
    string += " & ".join(["( (X{0} sub X & X{0} sub X{1} & X{0} ~= X{1}) => X{1} sub X)".format(i, i+1) for i in range(1, n)]) + ";"
    return string


def generate_horn_sub_even_alts(n, alt):
    if n < alt+1:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n" + "ex2 X: "
    for i in range(1, alt+1):
        if i % 2 == 0:
            string += "ex2"
        else:
            string += "all2"
        string += " X{}: ".format(i) if i != alt else " "
    string += ", ".join(["X" + str(i) for i in range(alt, n+1)]) + ": ~("
    string += " & ".join(["( (X{0} sub X & X{0} sub X{1} & X{0} ~= X{1}) => X{1} sub X)".format(i, i+1) for i in range(1, n)]) + ");"
    return string


def generate_veanes(n, alt):
    if n < alt:
        print("[*] Skipping n = {}".format(n))
        return None
    string = "ws1s;\n"
    string += "".join("ex1 x{}: ".format(str(i)) for i in range(1, n - alt + 1))
    string += "".join("~ex1 x{}: ".format(str(i)) for i in range(n-alt+1, n+1))
    string += " & ".join(["x{0} < x{1}".format(i, i + 1) for i in range(1, n)])
    string += ";"
    return string


def generate_veanes_ml(n, alt):
    if n < alt or n < 3:
        print("[*] Skipping n = {}".format(n))
        return None;
    string = ""
    string += "".join("(Ex (FO, ".format() for i in range(1, n - alt + 1))
    string += "".join("(Not (Ex (FO, ".format() for i in range(n-alt+1, n+1))
    base = "And(Less ({0}, {1}), Less({1}, {2}))".format(n-3, n-2, n-1)
    i = n - 4
    while i >= 0:
        base = "And(Less ({0}, {1}), ".format(i, i+1) + base + ")"
        i -= 1
    string += base
    string += ")"*( (n - alt)*2 + alt*3)
    return string


def generate_formulae(options, benchmark_name, up_to, alts, generator, zeroFill):
    for i in range(1, up_to + 1):
        if alts == 0 and benchmark_name != "veanes" and benchmark_name != "veanes_ml":
            formula = generator(i)
        else:
            formula = generator(i, alts)
        if formula is not None:
            output_name = benchmark_name + str(i).zfill(zeroFill) + ("_{}alts".format(alts) if alts != 0 else "") + ".mona"
            output_path = os.path.join(options.dir, output_name)
            with open(output_path, 'w') as file:
                file.write(formula)

if __name__ == '__main__':
    print("[*] WSkS Test&Benchmark Generator")
    print("[c] Tomas Fiedor, ifiedortom@fit.vutbr.cz")

    options = parse_arguments()

    # we will generate stuff
    generate_alternating = options.generate_alt is not None
    if not generate_alternating:
        print("[*] Generating benchmarks '{}' up to parameter n = {}".format(options.generate[0], options.generate[1]))
    else:
        print("[*] Generating benchmarks '{}' up to parameter n = {} with {} alternations".format(options.generate_alt[0], options.generate_alt[1], options.generate_alt[2]))
    benchmark_name = options.generate[0] if not generate_alternating else options.generate_alt[0]
    up_to = int(options.generate[1]) if not generate_alternating else int(options.generate_alt[1])
    alts = 0 if not generate_alternating else int(options.generate_alt[2])

    try:
        if generate_alternating and benchmark_name != "veanes" and benchmark_name != "veanes_ml":
            method_name = "generate_" + benchmark_name + "_"
            if alts % 2 == 0:
                method_name += "even_alts"
            else:
                method_name += "odd_alts"
        else:
            method_name = "generate_" + benchmark_name
        generator = getattr(sys.modules[__name__], method_name)
    except AttributeError:
        print("[!] No benchmark template for '{}'".format(benchmark_name))
        quit()
    zeroFill = len(str(up_to))
    zeroFill = 2 if zeroFill < 2 else zeroFill

    generate_formulae(options, benchmark_name, up_to, alts, generator, zeroFill)
