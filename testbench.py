'''
    WSkS Test Bench

    @author: Tomas Fiedor, ifiedortom@fit.vutbr.cz
    @summary: Test Bench script for running several benchmarks on binaries

'''

import argparse
import itertools
import os
import os.path
import pickle
import re
import subprocess
import sys
import smtplib
import time
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

dwina_error = -5
subprocess_error = -4
unknown_error = -3
timeout_error = -2
mona_error = -1                     # BDD Too Large for MONA
no_error = 0

errors = {
    -5: 'gaston fault',
    -4: 'subprocess_error',
    -3: 'unknown error',
    -2: 'timeout',
    -1: 'BDD too large',
    0: 'no error'
}

Measure = namedtuple('Measure', 'regex default post_process is_cummulative')
time_default = "0"
time_regex = "([0-9][0-9]:[0-9][0-9]:[0-9][0-9].[0-9][0-9])"
space_default = "0"
space_regex = "([0-9]+)"
double_regex = "([0-9]+\.[0-9]+)"
double_default = 1.0
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
            'space-mona': Measure("mona state space" + whatever_regex + space_regex, space_default, int, False),
            'space-all': Measure("overall state space" + whatever_regex + space_regex, space_default, int, False),
            'space-fix': Measure("fixpoints" + whatever_regex + space_regex, space_default, int, False),
            'space-base': Measure("bases" + whatever_regex + space_regex, space_default, int, False),
            'space-cont': Measure("continuations" + whatever_regex + space_regex, space_default, int, False),
            'space-prod': Measure("products" + whatever_regex + space_regex, space_default, int, False),
            'space-ternary': Measure("ternaryproducts" + whatever_regex + space_regex, space_default, int, False),
            'time-dp': Measure("decision procedure" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-base': Measure("dfa creation" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-conv': Measure("mona <-> vata" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-pre': Measure("preprocessing" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'time-sa': Measure("sa creation" + whatever_regex + time_regex, time_default, parse_total_time, False),
            'dag-nodes': Measure("nodes" + whatever_regex + space_regex, space_default, int, False),
            'real-nodes': Measure("real nodes" + whatever_regex + space_regex, space_default, int, False),
            'dag-gain': Measure("dag gain" + whatever_regex + double_regex, double_default, float, False),
            'form-vars': Measure("vars" + whatever_regex + space_regex, space_default, int, False),
            'form-atoms': Measure("atoms" + whatever_regex + space_regex, space_default, int, False),
            'aut-fix': Measure("fixpoint computations" + whatever_regex + space_regex, space_default, int, False),
            'aut-max-fix-nest': Measure("maximal fixpoint nesting" + whatever_regex + space_regex, space_default, int, False),
            'aut-height': Measure("automaton height" + whatever_regex + space_regex, space_default, int, False),
            'aut-max-refs': Measure("maximal references" + whatever_regex + space_regex, space_default, int, False),
            'cont-eval': Measure("evaluated" + whatever_regex + space_regex, space_default, int, False),
            'cont-eval-in-sub': Measure("in subsumption" + whatever_regex + space_regex, space_default, int, False),
            'cont-eval-in-isect': Measure("in isect nonempty" + whatever_regex + space_regex, space_default, int, False),
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

csv_keys = {
    'gaston': ['ret', 'time', 'space-all', 'space', 'space-mona', 'space-base', 'aut-height'],
    'mona': ['time', 'space', 'space-min']
}


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


def create_argument_parser():
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
    parser.add_argument('--message', '-m', default=None, help='additional message printed into testbench output')
    parser.add_argument('--csv-name', '-c', default=None, help='name of the output csv file')
    parser.add_argument('--repeat', '-r', action='store_true', help='repeat the last experiment with the exact same options')
    parser.add_argument('--gaston-params', '-gp', action='append', default=[], help='feeds additional params to gaston')
    parser.add_argument('--fixpoint-experiment', '-fe', action='store_true', help='runs gaston with various -cf parameters')
    return parser


def run_mona(test, timeout, params=[]):
    '''
    Runs MONA with following arguments:
    '''
    args = ['mona', '-s', '-t', '-q', '"{}"'.format(test)]
    output, retcode = runProcess(args, timeout)
    if(retcode != 0):
        if(retcode == 124):
            return timeout_error, ""
        elif(re.search("BDD too large", "\n".join(output)) is not None):
            return mona_error, ""
        else:
            return unknown_error, ""
    return parseMonaOutput(output)


def retcode_to_error(retcode):
    """
    :returns error code for output
    """
    if retcode == 0:
        return 0
    elif retcode == 124:
        return timeout_error
    else:
        return dwina_error


def run_gaston(test, timeout, params=[]):
    '''
    Runs dWiNA with following arguments: --method=backward
    '''
    args = ['./build/gaston', '--no-automaton'] + params + ['"{}"'.format(test)]
    output, retcode = runProcess(args, timeout)
    return parse_gaston_output(output, retcode_to_error(retcode))


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
            proc = subprocess.Popen(" ".join([timeout] + args), shell=True, stdout=subprocess.PIPE)
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


def exportToCSV(data, bins, options):
    '''
    Exports data to csv file

    data should be like this:
    data[benchmark]['mona'] = (time, space)
                   ['mona-expnf'] = (time, space, prefix-space)
                   ['gaston'] = (time-overall, time-dp-only, time-dfa, time-conversion, base-aut, space, space-unpruned)
                   ['gaston-dfa'] = (time, time-dp-only, time-dfa, time-conversion, base-aut, space, space-unpruned)

    '''
    saveTo = generateCSVname() if options.csv_name is None else options.csv_name
    if not saveTo.endswith('.csv'):
        saveTo = saveTo + ".csv"
    keys = []
    with open(saveTo, 'w') as csvFile:
        if options.message is not None:
            csvFile.write('"')
            csvFile.write(options.message)
            csvFile.write('"\n')
        # header of the file
        csvFile.write('benchmark, ')
        first = True
        if 'mona' in bins:
            for key in csv_keys['mona']:
                keys.append(('mona', key))
                if first:
                    first = False
                else:
                    csvFile.write(", ")
                csvFile.write(key)
        csvFile.write(", ")
        first = True
        if 'gaston' in bins:
            for key in csv_keys['gaston']:
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
                    error_code = data[benchmark][bin]
                    error_message = errors[error_code] + ("" if error_code != timeout_error else "({}m)".format(options.timeout))
                    bench_list = bench_list + [error_message]
                else:
                    try:
                        if key == 'ret':
                            ret_code = data[benchmark][bin][key]
                            ret_message = errors[ret_code] + ("" if ret_code != timeout_error else "({}m)".format(options.timeout))
                            bench_list = bench_list + [ret_message]
                        else:
                            bench_list = bench_list + [str(data[benchmark][bin][key])]
                    except Exception as exc:
                        print(exc)
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

    s = smtplib.SMTP('kazi.fit.vutbr.cz')
    #s = smtplib.SMTP('localhost')

    s.sendmail(sender, [receiver], msg.as_string())
    s.quit()
    time.sleep(1)


def generateCSVname():
    '''
    Generates "unique" name for csv file

    @returned generated name yyyy.mm.dd-hh:mm-timing.csv
    '''
    today = datetime.today()
    return "{0:02}.{1:02}.{2:02}-{3:02}.{4:02}-timing.csv".format(today.year, today.month, today.day, today.hour, today.minute)


def parse_gaston_output(output, retcode):
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
    data['ret'] = retcode
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
    parser = create_argument_parser()
    if len(sys.argv) == 0:
        parser.print_help()
        quit()
    else:
        return parser.parse_args()


def get_fixpoint_number_for(benchmark):
    """
    Runs the Gaston in '--dry-run' mode and extract the maximal number of fixpoint nesting.
    Note that this is further used for experiment, when we run the formula numerous times
    with various -cfX values.

    @param benchmark: benchmark we dry running
    @return: maximal fixpoint nesting in the @p benchmark
    """
    try:
        assert not benchmark.endswith('_serialized.mona')
        args = ['./build/gaston', '--dry-run --serialize'] + ['"{}"'.format(benchmark)]
        output, _ = runProcess(args, "1")
        return int(parse_measure('gaston', 'aut-max-fix-nest', output))
    except Exception as e:
        print("[!] Exception caught: ", e)
        return -1


def get_serialized_benchmark_name(benchmark):
    return os.path.splitext(benchmark)[0] + "_serialized.mona"


def export_to_dat(data, benchmark):
    """
    Exports the computed @p data to the benchmark.dat file, that will be used
    for ploting by gnuplot script

    @param[in] data: computed data for ploting
    @param[in] benchmark: filename of the benchmark
    """
    dat_file = os.path.splitext(benchmark)[0] + '.dat'
    dat_string = "# -cf\ttime\tsize\n"

    for cf, cf_data in enumerate(data):
        dat_string += "{}\t{}\t{}\t{}\n".format(cf, cf_data['time'], cf_data['space-all'], cf_data['space'])

    with open(dat_file, 'w') as dat_handle:
        dat_handle.write(dat_string)


def create_gnuplot_script(data, mona_data, benchmark):
    """
    Creates a gnuplot script, that will plot the @p data into the graph.

    @param[in] benchmark: filename of the benchmark
    """
    max_time = max(cf_data['time'] for cf_data in data)
    max_time = max(max_time, mona_data[0]['time'], mona_data[1]['time'])
    max_space = max(cf_data['space-all'] for cf_data in data)
    max_space = max(max_space, mona_data[0]['space-min'], mona_data[1]['space-min'])

    style_sheet = {
        'label': {
            'offset': 'offset 0.7, 0.5',
        },
        'axis': {
            'style': 'lt 1 lw 1 lc rgb "black"',
        },
    }

    # create the script body
    benchmark = os.path.splitext(benchmark)[0]
    script  = '# [!] Gnuplot script generated by testbench.py\n'
    script += '# \n'
    script += '# Plots the graph of state space (in states) and time according to the\n'
    script += '# number of converted fixpoints by MONA.\n'
    script += 'set term pngcairo dashed\n'
    script += 'set output "{}.png"\n'.format(benchmark)

    script += 'set multiplot layout 2, 1 title "{0}" font ",14"\n'.format(benchmark)

    # styles
    script += 'set style line 1 lt 1 pt 7 lw 2\n'
    script += 'set style line 2 lt 2 lc rgb "black" lw 1\n'
    script += 'set style line 3 lt 2 lc rgb "black" lw 2\n'

    script += 'set autoscale\n'
    script += 'set xzeroaxis {0[axis][style]}\n'.format(style_sheet)
    script += 'set yzeroaxis {0[axis][style]}\n'.format(style_sheet)
    script += 'set tmargin 3\n'
    script += 'unset border\n'

    script += 'set key outside right center box\n'
    script += 'set ylabel "Time [s]"\n'
    script += 'set xrange [0:{}]\n'.format(len(data)-1) # Fixme: This should be something different
    script += 'set xtics border in 0, 1, {} nomirror\n'.format(len(data)-1)
    script += 'set ytics axis ({}, {}) nomirror\n'.format(mona_data[0]['time'], mona_data[1]['time'])
    script += 'set yrange [0.0:{}]\n'.format(max_time)
    script += 'plot "{0}.dat" using 1:2 title "Gaston" with linespoints ls 1, \\\n'.format(benchmark)
    script += '     "{0}.dat" using 1:2:2 notitle with labels {1[label][offset]} font "Times,12", \\\n'.format(benchmark, style_sheet)
    script += '     {0} title "MONA (ser)" ls 2, \\\n'.format(mona_data[0]['time'])
    script += '     {0} title "MONA" ls 3\n'.format(mona_data[1]['time'])

    script += 'set xlabel "Fixpoints Converted"\n'
    script += 'set ylabel "State space [states]"\n'
    script += 'set ytics axis ({}, {}) nomirror\n'.format(mona_data[0]['space-min'], mona_data[1]['space-min'])
    script += 'set yrange [0:{}]\n'.format(max_space)
    script += 'plot "{0}.dat" using 1:3 title "Gaston all" with linespoints ls 1,\\\n'.format(benchmark)
    script += '     "{0}.dat" using 1:3:(sprintf("%d", $3)) title "" with labels {1[label][offset]} font "Times,12", \\\n'.format(benchmark, style_sheet)
    script += '     "{0}.dat" using 1:4 title "Gaston" with linespoints ls 1,\\\n'.format(benchmark)
    script += '     "{0}.dat" using 1:4:(sprintf("%d", $4)) title "" with labels {1[label][offset]} font "Times,12", \\\n'.format(benchmark, style_sheet)
    script += '     {0} title "MONA (ser)" ls 2, \\\n'.format(mona_data[0]['space-min'])
    script += '     {0} title "MONA" ls 3\n'.format(mona_data[1]['space-min'])
    script += 'unset multiplot\n'

    # write to .dat file
    with open('{}.plot'.format(benchmark), 'w') as script_file:
        script_file.write(script)


def create_gnuplot_image(gnuplot_script):
    """
    Runs gnuplot on the @p benchmark plot script to generate the image

    @param[in] gnuplot_script: gnuplot script used to generate the image
    """
    assert gnuplot_script.endswith('.plot')

    proc = subprocess.Popen('gnuplot -e "" {}'.format(gnuplot_script), shell=True, stdout=subprocess.PIPE)
    proc.wait()


if __name__ == '__main__':
    options = parse_arguments()
    if options.repeat:
        with open('testbench.config', 'rb') as f:
            options = pickle.load(f)
    else:
        with open('testbench.config', 'wb') as f:
            pickle.dump(options, f)

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

    if options.fixpoint_experiment:
        print("[!] Running fixpoint experiment")
        for root, dirs, filenames in os.walk(wdir):
            for f in filenames:
                benchmark = os.path.join(root, f)
                if not benchmark.endswith('.mona'):
                    continue
                if benchmark.endswith('_serialized.mona'):
                    continue

                # obtain additional information for experiment
                fixpoint_number = get_fixpoint_number_for(benchmark)
                serialized_benchmark = get_serialized_benchmark_name(benchmark)
                assert fixpoint_number > 0

                print("[*] Running test bench:"),
                print(colored("'{}'".format(benchmark), "white", attrs=["bold"]))

                mona_data, mona_ret = run_mona(serialized_benchmark, options.timeout, options.gaston_params)
                mona_orig_data, mona_orig_ret = run_mona(benchmark, options.timeout, options.gaston_params)
                assert mona_ret == mona_orig_ret

                cf_data = []
                for cf in range(0, fixpoint_number+1):
                    print("[!] Running Gaston with -cf{}".format(cf))
                    try:
                        data, ret = run_gaston(benchmark, options.timeout, options.gaston_params + ['-cf{}'.format(cf)])
                        cf_data.append(data)
                    except Exception as e:
                        data, ret = subprocess_error, ""

                    if mona_ret.upper() != ret:
                        print("\t->"),
                        print(colored("FAIL", "red")),
                        print("; Formula is "),
                        print(colored("'{}'".format(mona_ret), "white")),
                        print(" (gaston returned "),
                        print(colored("'{}'".format(ret.lower()), "white")),
                        print(")")
                        break
                else:
                    print("\t->"),
                    print(colored("OK", "green")),
                    print("; Formula is"),
                    print(colored("'{}'".format(mona_ret), "white"))

                    export_to_dat(cf_data, benchmark)
                    create_gnuplot_script(cf_data, (mona_data, mona_orig_data), benchmark)
                    create_gnuplot_image(os.path.splitext(benchmark)[0] + '.plot')
            if options.notify:
                break # Fixme: there should be img to be sent
                notify(csv, notified_out.getvalue())
                sys.stdout = saveout
    else:
        for root, dirs, filenames in os.walk(wdir):
            for f in filenames:
                benchmark = os.path.join(root, f)
                if not benchmark.endswith('.mona'):
                    continue
                data[benchmark] = {}
                tags = getTagsFromString(benchmark)
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
                        data[benchmark][bin], rets[bin] = method_call(benchmark, options.timeout, options.gaston_params)
                    except Exception as e:
                        print(e)
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
            csv = exportToCSV(data, bins, options)
            if options.notify:
                notify(csv, notified_out.getvalue())
                sys.stdout = saveout
