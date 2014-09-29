'''
    WSkS Test Bench

    @author: Tomas Fiedor, ifiedortom@fit.vutbr.cz
    @summary: Test Bench script for running several benchmarks on binaries
'''

import argparse
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
    parser.add_argument('--bin', '-b', default='./mona', help='binary that will be used for executing script')
    return parser

def runTest(test, binary, args):
    '''
    Runs single test for binary with arguments
    
    @param test: filename with test
    @param binary: binary we will be running
    @param args: arguments we will be feeding to binary
    '''
    pass

def getTagsFromString(string):
    '''
    Finds all tags from file
    
    @param string: string we are getting tags from
    @return: list of string tags
    '''
    tags = re.findall("[[][a-zA-Z0-9]+[]]", string)
    return [tag[1:-1] for tag in tags]

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
    
    # iterate through all files in dir
    executing_string = options.bin
    for root, dirs, filenames in os.walk(options.dir):
        for f in filenames:
            benchmark = os.path.join(root, f)
            tags = getTagsFromString(benchmark)
            if not benchmark.endswith('.mona'):
                continue
            # skips some benchmarks according to the tag
            if any([tag in options.skip for tag in tags]):
                continue
            # skips benchmarks that are not specified by only
            if options.only is not None and re.search(options.only, benchmark) is None:
                continue
            args = (options.bin, benchmark)
            # executing process
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()
            output = popen.stdout.readlines()
            
            #printing out
            times = [line for line in output if line.startswith('Total time:')]
            for time in times:
                print(benchmark + ": " + time[:-1])
            