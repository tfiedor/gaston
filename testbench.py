'''
    WSkS Test Bench

    @author: Tomas Fiedor, ifiedortom@fit.vutbr.cz
    @summary: Test Bench script for running several benchmarks on binaries
'''

import argparse
from datetime import datetime
import os
import re
import subprocess
import sys

def createArgumentParser():
    '''
    Creates Argument Parser object
    '''
    parser = argparse.ArgumentParser("WSkS Test Bench")
    parser.add_argument('--dir', '-d', default=(os.path.join(os.curdir, "benchmarks")), help="directory with benchmarks")
    parser.add_argument('--skip', '-s', action='append', default=['ws2s'], help='skips benchmarks with tag [SKIP]')
    parser.add_argument('--only', '-o', default=None, help='only test the benchmarks containing string ONLY')
    parser.add_argument('--bin', '-b', default=None, help='binary that will be used for executing script')
    return parser

def run_mona(test):
    '''
    Runs MONA with following arguments:
    '''
    args = ('./mona', '-s', test)
    output = runProcess(args)
    return parseMonaOutput(output)

def run_dwina(test):
    '''
    Runs dWiNA with following arguments: --method=backward
    '''
    args = ('./dWiNA', '--method=backward', test)
    args2 = ('./dWiNA-no-prune', '--method=backward', test)
    output = runProcess(args)
    output2 = runProcess(args2)
    return parsedWiNAOutput(output, output2)

def run_dwina_dfa(test):
    '''
    Runs dWiNA with following arguments: --method=backward --use-mona-dfa
    '''
    args =('./dWiNA', '--method=backward', '--use-mona-dfa', test)
    args2 = ('./dWiNA-no-prune', '--method=backward', '--use-mona-dfa', test)
    output = runProcess(args)
    output2 = runProcess(args2)
    return parsedWiNAOutput(output, output2)

def run_mona_expnf(test):
    '''
    Runs raped MONA with following arguments:
    '''
    args = ('./mona-expnf', '-s', test)
    output = runProcess(args)
    return parseMonaOutput(output)

def runProcess(args):
    '''
    Opens new subprocess and runs the arguments 
    
    @param: arguments to be run in subprocess
    @return read output
    '''
    return subprocess.Popen(" ".join(args), shell=True, stdout=subprocess.PIPE).stdout.readlines()

def getTagsFromString(string):
    '''
    Finds all tags from file
    
    @param string: string we are getting tags from
    @return: list of string tags
    '''
    tags = re.findall("[[][a-zA-Z0-9]+[]]", string)
    return [tag[1:-1] for tag in tags]

def exportToCSV(data):
    '''
    Exports data to csv file
    
    data should be like this:
    data[benchmark]['mona'] = (time, space)
                   ['mona-expnf'] = (time, space)
                   ['dwina'] = (time, time-dp-only, space, space-pruned)
                   ['dwina-dfa'] = (time, time-dp-only, space, space-pruned) 
    
    '''
    saveTo = generateCSVname()
    with open(saveTo, 'w') as csvFile:
        # header of the file
        csvFile.write('benchmark, '
                      'mona-time, mona-space,'                  # MONA time 
                      'mona-expnf-time, mona-expnf-space, '     # MONA spice
                      'dwina-time, dwina-time-dp-only, dwina-space, dwina-space-pruned,'
                      'dwina-dfa-time, dwina-dfa-time-dp-only, dwina-dfa-space, dwina-dfa-space-pruned\n')
        for benchmark in data.keys():
            csvFile.write(benchmark + ", ")
            for bin in ['mona', 'mona-expnf', 'dwina', 'dwina-dfa']:
                for i in range(0, len(data[benchmark][bin])):
                    csvFile.write(str(data[benchmark][bin][i]) + ", ")
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
    return 3600*float(match.group(1)) + 60*float(match.group(2)) + float(match.group(3))

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
    
    # get total time
    times = [line for line in strippedLines if line.startswith('Total time:')]
    
    assert(len(times) == 1)
    time = parseTotalTime(times[0])
    
    # filter out half of the output till the crap
    index = 0
    while not strippedLines[index].startswith('Projecting'):
        index = index + 1
    strippedLines = strippedLines[index:]
    
    # get all minimizings
    minimizations = [line for line in strippedLines if line.startswith('Minimizing')]    
    automata_sizes = [int((re.search('\(([0-9]+),[0-9]+\)', min)).group(1)) for min in minimizations]
    output_size = sum(automata_sizes)

    return (time, output_size)

def parsedWiNAOutput(output, unprunedOutput):
    '''
    
    @param output: lines with dwina output
    '''
    strippedLines = [line.lstrip() for line in output]
    
    # get total time
    times = [line for line in strippedLines if line.startswith('[*] Total elapsed time:')]
    assert(len(times) == 1)
    time = parseTotalTime(times[0])
    
    # get dp time
    times = [line for line in strippedLines if line.startswith('[*] Decision procedure elapsed time:')]
    assert(len(times) == 1)
    time_dp = parseTotalTime(times[0])
    
    # get size of state
    sizes = [line for line in strippedLines if line.startswith('[*] Size of the searched space:')]
    assert(len(sizes) == 1)
    size = int(re.search('[0-9]+', sizes[0]).group(0))
    
    strippedLines = [line.lstrip() for line in unprunedOutput]
    sizes = [line for line in strippedLines if line.startswith('[*] Size of the searched space:')]
    assert(len(sizes) == 1)
    size_unpruned = int(re.search('[0-9]+', sizes[0]).group(0))
    
    return (time, time_dp, size, size_unpruned)

def parseArguments():
    '''
    Parse input arguments
    '''
    parser = createArgumentParser()
    if len(sys.argv) == 0:
        parser.print_help()
        quit()
    else:
        return parser.parse_args()

if __name__ == '__main__':
    print("[*] WSkS Test Bench")
    print("[c] Tomas Fiedor, ifiedortom@fit.vutbr.cz")
    
    options = parseArguments()
    data = {}
    bins = ['dwina', 'dwina-dfa', 'mona', 'mona-expnf'] if (options.bin is None) else [options.bin] 
    
    # iterate through all files in dir
    executing_string = options.bin
    for root, dirs, filenames in os.walk(options.dir):
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
            
            print("[*] Running test bench: '{}'".format(benchmark))
            for bin in bins:
                method_name = "_".join(["run"] + bin.split('-'))
                method_call = getattr(sys.modules[__name__], method_name)
                data[benchmark][bin] = method_call(benchmark)
    exportToCSV(data)
            