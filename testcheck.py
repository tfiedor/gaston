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

dwina_error = (-1, -1, -1, -1, -1, -1, -1)
timeout_error = (-2, -2, -2, -2, -2, -2, -2)
mona_error = (-1, -1)
mona_expnf_error = (-1, -1, -1)


def createArgumentParser():
    '''
    Creates Argument Parser object
    '''
    parser = argparse.ArgumentParser("WSkS Test Check")
    parser.add_argument('--dir', '-d', default="basic", help="directory with benchmarks")
    parser.add_argument('--timeout', '-t', default=None, help='timeouts in minutes')
    parser.add_argument('--check', '-c', action='store_true', help='run the regression testing')
    parser.add_argument('--check-mem', '-cm', action='store_true', help='runs the valgrind during testing as well')
    return parser


def run_mona(test, timeout, checkonly=False):
    '''
    Runs MONA with following arguments:
    '''
    args = ('mona', '-s', '"{}"'.format(test))
    output, retcode = runProcess(args, timeout)
    if(retcode != 0):
        return mona_error, output
    return parseMonaOutput(output, False, checkonly)


def run_gaston(test, timeout, checkonly=False):
    '''
    Runs dWiNA with following arguments: --method=backward
    '''
    args = ('./gaston', '--test=val', '"{}"'.format(test))
    output, retcode = runProcess(args, timeout)

    # Fixme: This should be the issue of segfault
    if (retcode != 0):
        if(retcode == 124):
            return timeout_error, ""
        else:
            return dwina_error, ""
    return parsedWiNAOutput(output, "", checkonly)


def run_valgrind(test, timeout):
    '''
    Runs gaston on valgrind to check for leaks
    :param test:
    :param timeout:
    :return:
    '''
    args = ('valgrind', './gaston', '"{}"'.format(test))
    output, retcode = runProcess(args, timeout, True)

    return parse_valgrind_output(output)


def runProcess(args, timeout, from_error=False):
    '''
    Opens new subprocess and runs the arguments

    @param: arguments to be run in subprocess
    @return read output
    '''
    if from_error:
        proc = subprocess.Popen(" ".join(args), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = proc.stderr.readlines()
        proc.wait()
        return (output, proc.returncode)
    else:
        timeout = "timeout {0}m".format(timeout) if (timeout is not None) else None
        if timeout is None:
            proc = subprocess.Popen(" ".join(args), shell=True, stdout=subprocess.PIPE)
        else:
            proc = subprocess.Popen(" ".join((timeout, ) + args), shell=True, stdout=subprocess.PIPE)
        output = proc.stdout.readlines()
        proc.wait()
        return (output, proc.returncode)


def parseMonaOutput(output, isExPNF, checkonly=False):
    '''
    Gets mona or mona-expnf output, strips all whitespaces from start then
    gets a line with "Total time:", parses the time in seconds, in float,
    filters out all the automata until first projection and then gets all
    automata that are minimized and summed up to get the number of states that
    mona generates

    @param output: lines with mona output
    '''
    strippedLines = [line.lstrip() for line in output]

    ret = ""
    for line in strippedLines:
        match = re.search("Formula is ([a-zA-Z]+)", line)
        if match is not None:
            ret = match.group(1)
            break
        match = re.search("A satisfying example", line)
        if match is not None:
            ret = "satisfiable"
            break

    return (-1, -1), ret


def parsedWiNAOutput(output, unprunedOutput, checkonly=False):
    '''

    @param output: lines with dwina output
    '''
    strippedLines = [line.lstrip() for line in output]
    ret = ""
    for line in strippedLines:
        match = re.search("\[!\] Formula is [^']*'([A-Z]+)'[^']*", line)
        if match is not None:
            ret = match.group(1)
            break

    return (-1, -1, -1, -1, -1), ret


def parse_valgrind_output(output):
    '''

    :param output:
    :return:
    '''
    stripped_lines = [line.lstrip() for line in output]
    for line in stripped_lines:
        match = re.search(".*total heap usage: ([0-9]*(,[0-9]*)*) allocs, ([0-9]*(,[0-9]*)*) frees, ([0-9]*(,[0-9]*)*) bytes allocated", line)
        if match is not None:
            return match.group(1), match.group(3), match.group(5)


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

if __name__ == '__main__':
    print("[*] WSkS Test Bench")
    print("[c] Tomas Fiedor, ifiedortom@fit.vutbr.cz")

    options = parse_arguments()

    # we will generate stuff
    data = {}
    # modification and setup of parameters
    bins = ['mona', 'gaston']
    wdir = os.path.join(os.curdir, "tests", options.dir)

    # iterate through all files in dir
    cases = 0
    all_cases = []
    fails = 0
    failed_cases = []
    for root, dirs, filenames in os.walk(wdir):
        for f in filenames:
            benchmark = os.path.join(root, f)
            data[benchmark] = {}
            if not benchmark.endswith('.mona'):
                continue

            print("[*] Running test bench:"),
            print(colored("'{}'".format(benchmark), "white", attrs=["bold"]))
            rets = {'gaston': ""}
            for bin in bins:
                method_name = "_".join(["run"] + bin.split('-'))
                method_call = getattr(sys.modules[__name__], method_name)
                data[benchmark][bin], rets[bin] = method_call(benchmark, options.timeout, options.check)
            allocs, frees, bytes = "", "", ""
            if options.check_mem:
                allocs, frees, bytes = run_valgrind(benchmark, options.timeout)
            cases += 1
            if rets['mona'] == -1 or rets['mona'] == "":
                print("\t-> MONA failed or could not be determined")
            elif rets['mona'].upper() != rets['gaston']:
                print("\t->"),
                print(colored("FAIL", "red")),
                print("; Formula is "),
                print(colored("'{}'".format(rets['mona']), "white")),
                print(" (gaston returned "),
                print(colored("'{}'".format(rets['gaston'].lower()), "white")),
                print(")")
                fails += 1
                failed_cases.append("'{}': gaston ('{}') vs mona ('{}')".format(benchmark, rets['gaston'], rets['mona'].lower()))
                all_cases.append("FAIL : '{}': gaston ('{}') vs mona ('{}')".format(benchmark, rets['gaston'], rets['mona'].lower()))
            elif allocs != frees:
                print("\t->"),
                print(colored("FAIL", "red")),
                print("; Memcheck found leaks: "),
                print(" {} allocs / {} frees (out of {} allocated)".format(allocs, frees, bytes))
                fails += 1
                failed_cases.append("'{}': gaston leaks: {} allocs / {} frees".format(benchmark, allocs, frees))
                all_cases.append("FAIL : '{}': gaston leaks: {} allocs / {} frees".format(benchmark, allocs, frees))
            else:
                all_cases.append("OK : '{}': gaston ('{}') vs mona ('{}')".format(benchmark, rets['gaston'], rets['mona'].lower()))
                print("\t->"),
                print(colored("OK", "green")),
                print("; Formula is"),
                print(colored("'{}'".format(rets['mona']), "white")),
                print(" ({} allocated)".format(allocs, frees, bytes))

    print("[*] Running statistics of tests:")
    print("[!] "),
    clr = "red" if cases-fails != cases else "green"
    print(colored("{0}/{1} passes".format(cases-fails, cases), clr, attrs=["bold"]))
    print("[!] Regression tests "),
    with open('testbench.log', 'w') as ac_file:
        ac_file.write("\n".join(all_cases))
    if cases-fails != cases:
        print(colored("failed", "red", attrs=["bold"]))
        print("[!] Saving failed cases to :"),
        print(colored("'testbench-fail.log'", "grey", attrs=["bold"]))
        with open('testbench-fail.log', 'w') as fc_file:
            fc_file.write("\n".join(failed_cases))
    else:
        print(colored("passed", "green", attrs=["bold"]))
