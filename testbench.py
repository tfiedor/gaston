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
    parser = argparse.ArgumentParser("WSkS Test Bench")
    parser.add_argument('--dir', '-d', default=(os.path.join(os.curdir, "benchmarks")), help="directory with benchmarks")
    parser.add_argument('--skip', '-s', action='append', default=['ws2s'], help='skips benchmarks with tag [SKIP]')
    parser.add_argument('--only', '-o', default=None, help='only test the benchmarks containing string ONLY')
    parser.add_argument('--bin', '-b', action='append', default=None, help='binary that will be used for executing script')
    parser.add_argument('--no-export-to-csv', '-x', action='store_true', help='will not export to csv')
    parser.add_argument('--timeout', '-t', default=None, help='timeouts in minutes')
    return parser


def run_mona(test, timeout):
    '''
    Runs MONA with following arguments:
    '''
    args = ('mona', '-s', '"{}"'.format(test))
    output, retcode = runProcess(args, timeout)
    if(retcode != 0):
        return mona_error, output
    return parseMonaOutput(output)


def run_gaston(test, timeout):
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
    return parsedWiNAOutput(output, "")


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


def getTagsFromString(string):
    '''
    Finds all tags from file

    @param string: string we are getting tags from
    @return: list of string tags
    '''
    tags = re.findall("[[][a-zA-Z0-9]+[]]", string)
    return [tag[1:-1] for tag in tags]


def exportToCSV(data, bins):
    '''
    Exports data to csv file

    data should be like this:
    data[benchmark]['mona'] = (time, space)
                   ['mona-expnf'] = (time, space, prefix-space)
                   ['gaston'] = (time-overall, time-dp-only, time-dfa, time-conversion, base-aut, space, space-unpruned)
                   ['gaston-dfa'] = (time, time-dp-only, time-dfa, time-conversion, base-aut, space, space-unpruned)

    '''
    saveTo = generateCSVname()
    with open(saveTo, 'w') as csvFile:
        # header of the file
        csvFile.write('benchmark, ')
        if 'mona' in bins:
            csvFile.write('mona-time, mona-space, ')
        if 'gaston' in bins:
            csvFile.write('gaston-time-overall, gaston-time-dp-only, gaston-time-dfa, gaston-time-conversion, base-aut, gaston-space, gaston-space-pruned, ')
        csvFile.write('\n')

        for benchmark in sorted(data.keys()):
            bench_list = [os.path.split(benchmark)[1]]
            for bin in bins:
                for i in range(0, len(data[benchmark][bin])):
                    bench_list = bench_list + [str(data[benchmark][bin][i])]
            csvFile.write(", ".join(bench_list))
            csvFile.write('\n')


def generateCSVname():
    '''
    Generates "unique" name for csv file

    @returned generated name yyyy.mm.dd-hh:mm-timing.csv
    '''
    today = datetime.today()
    return "{0:02}.{1:02}.{2:02}-{3:02}.{4:02}-timing.csv".format(today.year, today.month, today.day, today.hour, today.minute)


def parseTotalTime(line):
    '''
    @param line: time line in format 'Total time: 00:00:00.00'
    @return: time in seconds, in float
    '''
    match = re.search("([0-9][0-9]):([0-9][0-9]):([0-9][0-9].[0-9][0-9])", line)
    time = 3600*float(match.group(1)) + 60*float(match.group(2)) + float(match.group(3))
    return time if time != 0 else 0.01


def parsedWiNAOutput(output, unprunedOutput):
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

    # get total time
    times = [line for line in strippedLines if line.startswith('[*] Total elapsed time:')]
    if (len(times) != 1):
        return (-1, -1, -1, -1, -1), ret
    time = parseTotalTime(times[0])

    # get size of state
    sizes = [line for line in strippedLines if line.startswith('[*] Overall State Space:')]
    size = int(re.search('[0-9]+', sizes[0]).group(0))

    sizes = [line for line in strippedLines if line.startswith('[*] Explored Fixpoint Space:')]
    size_unpruned = int(re.search('[0-9]+', sizes[0]).group(0))

    # get dp time
    times = [line for line in strippedLines if line.startswith('[*] Decision procedure:')]
    time_dp = parseTotalTime(times[0])

    # get dp time
    times = [line for line in strippedLines if line.startswith('[*] DFA creation:')]
    time_dfa = parseTotalTime(times[0])

    # get dp time
    times = [line for line in strippedLines if line.startswith('[*] MONA <-> VATA:')]
    time_conv = parseTotalTime(times[0])

    return (time, time_dp, time_dfa, time_conv, 0, size, size_unpruned), ret


def parseMonaOutput(output):
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

    # get total time
    times = [line for line in strippedLines if line.startswith('Total time:')]

    if len(times) != 1:
        return mona_error
    time = parseTotalTime(times[0])

    # get all minimizings
    minimizations = [line for line in strippedLines if line.startswith('Minimizing')]
    automata_sizes = [int((re.search('\(([0-9]+),[0-9]+\)', min)).group(1)) for min in minimizations]
    output_size = sum(automata_sizes)

    return (time, output_size), ret


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
    if options.bin is not None and len(options.bin) > 4:
        print("[!] Invalid number of binaries")
        quit()
    bins = ['mona', 'gaston'] if (options.bin is None) else options.bin
    wdir = options.dir

    # iterate through all files in dir
    executing_string = options.bin
    cases = 0
    all_cases = []
    fails = 0
    failed_cases = []
    for root, dirs, filenames in os.walk(wdir):
        for f in filenames:
            benchmark = os.path.join(root, f)
            data[benchmark] = {}
            tags = getTagsFromString(benchmark)
            if not benchmark.endswith('.mona'):
                continue
            # skips some benchmarks according to the tag
            if any([tag in options.skip for tag in tags]):
                continue
            # skips benchmarks that are not specified by only
            if options.only is not None and re.search(options.only, benchmark) is None:
                continue

            print("[*] Running test bench:"),
            print(colored("'{}'".format(benchmark), "white", attrs=["bold"]))
            rets = {'gaston': ""}
            for bin in bins:
                method_name = "_".join(["run"] + bin.split('-'))
                method_call = getattr(sys.modules[__name__], method_name)
                data[benchmark][bin], rets[bin] = method_call(benchmark, options.timeout)

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
            else:
                all_cases.append("OK : '{}': gaston ('{}') vs mona ('{}')".format(benchmark, rets['gaston'], rets['mona'].lower()))
                print("\t->"),
                print(colored("OK", "green")),
                print("; Formula is"),
                print(colored("'{}'".format(rets['mona']), "white"))
    if not options.no_export_to_csv:
        exportToCSV(data, bins)
