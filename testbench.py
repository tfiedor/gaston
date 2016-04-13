'''
    WSkS Test Bench

    @author: Tomas Fiedor, ifiedortom@fit.vutbr.cz
    @summary: Test Bench script for running several benchmarks on binaries

'''

import argparse
import itertools
import os
import re
import subprocess
import sys
import smtplib
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email import Encoders
from datetime import datetime
from threading import Timer
from termcolor import colored
from collections import namedtuple
from cStringIO import StringIO

sender = 'ifiedortom@fit.vutbr.cz'
receiver = 'ifiedortom@fit.vutbr.cz'

subprocess_error = -4
unknown_error = -3
timeout_error = -2
mona_error = -1                     # BDD Too Large for MONA

Measure = namedtuple('Measure', 'regex default post_process is_cummulative')
time_default = "0"
time_regex = "([0-9][0-9]:[0-9][0-9]:[0-9][0-9].[0-9][0-9])"
space_default = "0"
space_regex = "([0-9]+)"
pair_regex = "\(([0-9]+),([0-9]+)\)"
whatever_regex = ".*?"

def parse_total_time(line):
    '''
    @param line: time line in format 'Total time: 00:00:00.00'
    @return: time in seconds, in float
    '''
    match = re.search("([0-9][0-9]):([0-9][0-9]):([0-9][0-9].[0-9][0-9])", line)
    time = 3600*float(match.group(1)) + 60*float(match.group(2)) + float(match.group(3))
    return time if time != 0 else 0.01

measures = {
    'gaston': {
            'symbols': Measure("symbols" + whatever_regex + space_regex, space_default, int, False),
            'time': Measure("total elapsed time" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'space': Measure("explored fixpoint space" + whatever_regex + space_regex, space_default, int, False),
            'space-all': Measure("overall state space" + whatever_regex + space_regex, space_default, int, False),
            'space-fix': Measure("fixpoints" + whatever_regex + space_regex, space_default, int, False),
            'space-bases': Measure("bases" + whatever_regex + space_regex, space_default, int, False),
            'space-continuations': Measure("continuations" + whatever_regex + space_regex, space_default, int, False),
            'space-products': Measure("products" + whatever_regex + space_regex, space_default, int, False),
            'time-dp': Measure("decision procedure" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-base': Measure("dfa creation" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-conv': Measure("mona <-> vata" + whatever_regex + time_regex, time_default, parse_total_time, False),
        },
    'mona': {
            'time': Measure("total time" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-min': Measure("minimize" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-project': Measure("project" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-product': Measure("product" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-copy': Measure("copy" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-replace': Measure("replace" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-saturate': Measure("right-quotient" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-negate': Measure("negate" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'space': Measure("minimizing" + whatever_regex + pair_regex + whatever_regex + pair_regex, space_default,
                lambda parsed: sum([int(item[0]) for item in parsed]), True),
            'space-min': Measure(whatever_regex + "minimizing" + whatever_regex + pair_regex + whatever_regex + pair_regex, space_default,
                lambda parsed: sum([int(item[2]) for item in parsed]), True),
            'bdd': Measure("minimizing" + whatever_regex + pair_regex + whatever_regex + pair_regex, space_default,
                lambda parsed: sum([int(item[1]) for item in parsed]), True),
            'bdd-min': Measure("minimizing" + whatever_regex + pair_regex + whatever_regex + pair_regex, space_default,
                lambda parsed: sum([int(item[3]) for item in parsed]), True)
    }
}
dwina_error = {key: -1 for key in measures['gaston'].keys()}


def parse_measure(tool, measure_name, input):
    measure = measures[tool][measure_name]
    if measure.is_cummulative:
        match = re.findall(measure.regex, "\n".join(input).lower())
    else:
        match = re.search(measure.regex, "\n".join(input).lower())
    if match is None or (isinstance(match, list) and len(match) == 0):
        return measure.default
    elif measure.is_cummulative:
        return measure.post_process(match)
    else:
        return measure.post_process(match.group(1))


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
    parser.add_argument('--notify', '-n', action='store_true', help="sends email notification to email")
    return parser


def run_mona(test, timeout):
    '''
    Runs MONA with following arguments:
    '''
    args = ('mona', '-s', '-t', '-q', '"{}"'.format(test))
    output, retcode = runProcess(args, timeout)
    if(retcode != 0):
        if(retcode == 124):
            return timeout_error, ""
        elif(re.search("BDD too large", "\n".join(output)) is not None):
            return mona_error, ""
        else:
            return unknown_error, ""
    return parseMonaOutput(output)


def run_gaston(test, timeout):
    '''
    Runs dWiNA with following arguments: --method=backward
    '''
    args = ('./gaston', '"{}"'.format(test))
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
    keys = []
    with open(saveTo, 'w') as csvFile:
        # header of the file
        csvFile.write('benchmark, ')
        first = True
        if 'mona' in bins:
            for (key, timing) in measures['mona'].items():
                keys.append(('mona', key))
                if first:
                    first = False
                else:
                    csvFile.write(", ")
                csvFile.write(key)
        csvFile.write(", ")
        first = True
        if 'gaston' in bins:
            for (key, timing) in measures['gaston'].items():
                keys.append(('gaston', key))
                if first:
                    first = False
                else:
                    csvFile.write(", ")
                csvFile.write(key)
        csvFile.write('\n')

        for benchmark in sorted(data.keys()):
            bench_list = [os.path.split(benchmark)[1]]
            for (bin, key) in keys:
                if not hasattr(data[benchmark][bin], "__getitem__"):
                    bench_list = bench_list + [str(data[benchmark][bin])]
                else:
                    try:
                        bench_list = bench_list + [str(data[benchmark][bin][key])]
                    except Exception:
                        print(bin, key, data[benchmark][bin])
            csvFile.write(", ".join(bench_list))
            csvFile.write('\n')
    return saveTo


def notify(results, contents):
    '''
    Sends the email notification together with results after the testbench terminates

    @:param result: contents of the generated csv, fil.
    '''
    msg = MIMEMultipart()

    msg['Subject'] = '[Testbench.py] Results of ' + results
    msg['From'] = sender
    msg['To'] = receiver

    msg.attach(MIMEText(contents))

    attachment = MIMEBase('application', 'octet-stream')
    attachment.set_payload(open(results, 'rb').read())
    Encoders.encode_base64(attachment)
    attachment.add_header('Content-Disposition', 'attachment; filename="{}"'.format(results))

    msg.attach(attachment)

    s = smtplib.SMTP('localhost')
    s.sendmail(sender, [receiver], msg.as_string())
    s.quit()

def generateCSVname():
    '''
    Generates "unique" name for csv file

    @returned generated name yyyy.mm.dd-hh:mm-timing.csv
    '''
    today = datetime.today()
    return "{0:02}.{1:02}.{2:02}-{3:02}.{4:02}-timing.csv".format(today.year, today.month, today.day, today.hour, today.minute)


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

    data = {}
    for (name, measure) in measures['gaston'].items():
        data[name] = parse_measure('gaston', name, strippedLines)

    return data, ret


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
    data = {}
    for (name, measure) in measures['mona'].items():
        data[name] = parse_measure('mona', name, strippedLines)

    return data, ret


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
    saveout = sys.stdout
    if options.notify:
        colored = lambda text, *params, **keywords: text
        sys.stdout = notified_out = StringIO()

    print("[*] WSkS Test Bench")
    print("[c] Tomas Fiedor, ifiedortom@fit.vutbr.cz")

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
                try:
                    method_call = getattr(sys.modules[__name__], method_name)
                    data[benchmark][bin], rets[bin] = method_call(benchmark, options.timeout)
                except Exception:
                    data[benchmark][bin], rets[bin] = subprocess_error, ""

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
        csv = exportToCSV(data, bins)
        if options.notify:
            notify(csv, notified_out.getvalue())
            sys.stdout = saveout
