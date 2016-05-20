#!/usr/bin/env python2
"""
    WSkS Test Bench

    @author: Tomas Fiedor, ifiedortom@fit.vutbr.cz
    @summary: Test Bench script for running several benchmarks on binaries

"""

import argparse
from datetime import datetime
import itertools
import os
import re
import subprocess
import sys
from threading import Timer
from termcolor import colored

dwina_error = -1
timeout_error = -2
mona_error = -1
mona_expnf_error = -2
time_error = 0.1        # error in percents for timing
reference_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), "tests", "perf", "reference.perf")


def createArgumentParser():
    """
    Creates Argument Parser object
    """
    parser = argparse.ArgumentParser("WSkS Test Check")
    parser.add_argument('--dir', '-d', default="basic", help="directory with benchmarks")
    parser.add_argument('--timeout', '-t', default=None, help='timeouts in minutes')
    parser.add_argument('--check', '-c', action='store_true', help='run the regression testing')
    parser.add_argument('--check-mem', '-cm', action='store_true', help='runs the valgrind during testing as well')
    parser.add_argument('--check-perf', action='store_true', help='run the performance testing')
    return parser


def run_mona(test, timeout, checkonly=False):
    """
    Runs MONA with following arguments:
    """
    args = ('mona', '-s', '"{}"'.format(test))
    output, retcode = runProcess(args, timeout)
    if(retcode != 0):
        return mona_error, output
    return parseMonaOutput(output, False, checkonly)


def run_gaston(test, timeout, checkonly=False):
    """
    Runs dWiNA with following arguments: --method=backward
    """
    gaston_bin = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'build/gaston')
    args = (gaston_bin, '"{}"'.format(test))
    output, retcode = runProcess(args, timeout)

    # Fixme: This should be the issue of segfault
    if retcode != 0:
        if retcode == 124:
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
        return output, proc.returncode
    else:
        timeout = "timeout {0}m".format(timeout) if (timeout is not None) else None
        if timeout is None:
            proc = subprocess.Popen(" ".join(args), shell=True, stdout=subprocess.PIPE)
        else:
            proc = subprocess.Popen(" ".join((timeout, ) + args), shell=True, stdout=subprocess.PIPE)
        output = proc.stdout.readlines()
        proc.wait()
        return output, proc.returncode


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


def parse_total_time(line):
    """
    @param line: time line in format 'Total time: 00:00:00.00'
    @return: time in seconds, in float
    """
    match = re.search("([0-9][0-9]):([0-9][0-9]):([0-9][0-9].[0-9][0-9])", line)
    time = 3600*float(match.group(1)) + 60*float(match.group(2)) + float(match.group(3))
    return time if time != 0 else 0.01


def parsedWiNAOutput(output, unprunedOutput, checkonly=False):
    """

    @param output: lines with dwina output
    """
    stripped_lines = [line.lstrip() for line in output]
    ret = ""
    time = -1
    for line in stripped_lines:
        match = re.search("\[!\] Formula is [^']*'([A-Z]+)'[^']*", line)
        if match is not None:
            ret = match.group(1)
            break
    for line in stripped_lines:
        match = re.search("\[*\] Total elapsed time: ", line)
        if match is not None:
            time = parse_total_time(line)
            break
    return time, ret


def parse_valgrind_output(output):
    """

    :param output:
    :return:
    """
    stripped_lines = [line.lstrip() for line in output]
    for line in stripped_lines:
        match = re.search(".*total heap usage: ([0-9]*(,[0-9]*)*) allocs, ([0-9]*(,[0-9]*)*) frees, ([0-9]*(,[0-9]*)*) bytes allocated", line)
        if match is not None:
            return match.group(1), match.group(3), match.group(5)


def parse_arguments():
    """
    Parse input arguments
    """
    parser = createArgumentParser()
    if len(sys.argv) == 0:
        parser.print_help()
        quit()
    else:
        opt = parser.parse_args()
        if opt.check_mem is True and opt.check_perf is True:
            print("Conflicting options --check-mem and --check-perf")
            parser.print_help()
            quit()
        elif opt.check_perf and opt.timeout is None:
            opt.timeout = "1"
        return opt


def get_benchmark_dir(opt):
    """
    Returns the dir with the benchmarks according to the set options

    :param opt  options of testcheck.py
    """
    benchmark_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "tests",
                                 opt.dir if opt.check_perf is False else "perf")
    return benchmark_dir


def load_performance_times():
    """
    :returns dictionary for benchmarks and times
    """
    with open(reference_file, 'r') as p:
        dict = {line.split(":")[0]: float(line.split(":")[1].strip()) for line in p.readlines() if ":" in line}
        return dict


def check_times(time, reference_time):
    """
    :returns (is performance degradation, is performance gain)
    """
    lower_bound = reference_time*(1-time_error)
    upper_bound = reference_time*(1+time_error)

    if time == -2:
        if reference_time == -2:
            return True, False, reference_time, reference_time
        else:
            return True, False, lower_bound, upper_bound

    if time > upper_bound:
        return True, False, lower_bound, upper_bound
    if time < lower_bound:
        return False, True, lower_bound, upper_bound
    else:
        return False, False, lower_bound, upper_bound


if __name__ == '__main__':
    print("[*] WSkS Test Bench")
    print("[c] Tomas Fiedor, ifiedortom@fit.vutbr.cz")

    options = parse_arguments()

    # we will generate stuff
    data = {}
    performance_reference = load_performance_times()

    # modification and setup of parameters
    bins = ['mona', 'gaston']
    wdir = get_benchmark_dir(options)

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
            if options.check_perf:
                print("\t->"),
                try:
                    is_degradation, is_gain, lower_bound, upper_bound \
                        = check_times(data[benchmark]['gaston'], performance_reference[benchmark])
                    if is_degradation:
                        print(colored("FAIL", "red")),
                        print(":"),
                        print(colored(data[benchmark]['gaston'], "red")),
                        print(" degrades from {0:.3f}".format(upper_bound))
                        fails += 1
                    else:
                        print(colored("OK", "green")),
                        print(":"),
                        print(colored(data[benchmark]['gaston'], "green")),
                        print(" within bounds ({0:.3f}, {1:.3f})".format(lower_bound, upper_bound))
                        performance_reference[benchmark] = \
                            (performance_reference[benchmark] + data[benchmark]['gaston']) / 2
                except KeyError as k:
                    print(colored("UNKNOWN", "yellow")),
                    print(":"),
                    print(data[benchmark]['gaston'])
                    performance_reference[benchmark] = data[benchmark]['gaston']
            elif rets['mona'] == -1 or rets['mona'] == "":
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
    with open(reference_file, 'w') as ref_file:
        ref_file.write("\n".join("{}:{}".format(key, value) for (key, value) in performance_reference.items()))
    if cases-fails != cases:
        print(colored("failed", "red", attrs=["bold"]))
        print("[!] Saving failed cases to :"),
        print(colored("'testbench-fail.log'", "grey", attrs=["bold"]))
        with open('testbench-fail.log', 'w') as fc_file:
            fc_file.write("\n".join(failed_cases))
    else:
        print(colored("passed", "green", attrs=["bold"]))
    exit(1)