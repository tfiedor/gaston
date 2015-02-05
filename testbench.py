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

dwina_error = (-1, -1, -1, -1, -1)
mona_error = (-1, -1)
mona_expnf_error = (-1, -1, -1)
test_dir="./tests/"

def createArgumentParser():
	'''
	Creates Argument Parser object
	'''
	parser = argparse.ArgumentParser("WSkS Test Bench")
	parser.add_argument('--dir', '-d', default=(os.path.join(os.curdir, "benchmarks")), help="directory with benchmarks")
	parser.add_argument('--skip', '-s', action='append', default=['ws2s'], help='skips benchmarks with tag [SKIP]')
	parser.add_argument('--only', '-o', default=None, help='only test the benchmarks containing string ONLY')
	parser.add_argument('--bin', '-b', action='append', default=None, help='binary that will be used for executing script')
	parser.add_argument('--generate', '-g', default=None, nargs=2, help='generates parametrized benchmark up to N')
	parser.add_argument('--generate-alt', '-a', default=None, nargs=3, help='generates parametrized benchmarks up to N with ALT alternation')
	parser.add_argument('--no-export-to-csv', '-x', action='store_true', help='will not export to csv')
	parser.add_argument('--timeout', '-t', default=None, help='timeouts in minutes')
	parser.add_argument('--check', '-c', action='store_true', help='no timing prints various statistics')
	return parser

def run_mona(test, timeout, checkonly=False):
	'''
	Runs MONA with following arguments:
	'''
	args = ('./mona', '-s', '"{}"'.format(test))
	output, retcode = runProcess(args, timeout)
	if(retcode != 0):
		print(output)
		return mona_error, ""
	return parseMonaOutput(output, False, checkonly)

def run_mona_expnf(test, timeout, checkonly=False):
	'''
	Runs raped MONA with following arguments:
	'''
	args = ('./mona-expnf', '-s', '"{}"'.format(test))
	output, retcode = runProcess(args, timeout)
	if(retcode != 0):
		return mona_expnf_error, ""
	return parseMonaOutput(output, True, checkonly)

def run_dwina(test, timeout, check, checkonly=False):
	'''
	Runs dWiNA with following arguments: --method=backward
	'''
	args = ('./dWiNA', '--method=backward', '"{}"'.format(test))
	args2 = ('./dWiNA-no-prune', '--method=backward', '"{}"'.format(test))
	output, retcode = runProcess(args, timeout)
	if checkonly:
		output2, retcode2 = "", 0
	else:	
		output2, retcode2 = runProcess(args2, timeout)
	if (retcode != 0) and (retcode2 != 0):
		return dwina_error, ""
	return parsedWiNAOutput(output, output2, checkonly)

def run_dwina_dfa(test, timeout, checkonly=False):
	'''
	Runs dWiNA with following arguments: --method=backward --use-mona-dfa
	'''
	args =('./dWiNA', '--method=backward', '--no-expnf', '"{}"'.format(test))
	args2 = ('./dWiNA-no-prune', '--method=backward', '--no-expnf', '"{}"'.format(test))
	output, retcode = runProcess(args, timeout)
	if checkonly:
		output2, retcode2 = "", 0
	else:
		output2, retcode2 = runProcess(args2, timeout)
	if (retcode != 0) and (retcode2 != 0):
		return dwina_error, ""
	return parsedWiNAOutput(output, output2, checkonly)


def runProcess(args, timeout):
	'''
	Opens new subprocess and runs the arguments 
	
	@param: arguments to be run in subprocess
	@return read output
	'''
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
				   ['dwina'] = (time, time-dp-only, base-aut, space, space-unpruned)
				   ['dwina-dfa'] = (time, time-dp-only, base-aut, space, space-unpruned) 
	
	'''
	saveTo = generateCSVname()
	with open(saveTo, 'w') as csvFile:
		# header of the file
		csvFile.write('benchmark, ')
		if 'mona' in bins:
			csvFile.write('mona-time, mona-space, ')
		if 'mona-expnf' in bins:
			csvFile.write('mona-expnf-time, mona-expnf-space, mona-expnf-prefix-space, ')
		if 'dwina' in bins:
			csvFile.write('dwina-time, dwina-time-dp-only, base-aut, dwina-space, dwina-space-pruned, ')
		if 'dwina-dfa' in bins:
			csvFile.write('dwina-dfa-time, dwina-dfa-time-dp-only, base-aut, dwina-dfa-space, dwina-dfa-space-pruned')
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
	return 3600*float(match.group(1)) + 60*float(match.group(2)) + float(match.group(3))

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

	# if checkonly, we only care about the sat/unsat/val
	if checkonly:
		if isExPNF:
			return (-1, -1, -1), ret
		else:
			return (-1, -1), ret
	
	# get total time
	times = [line for line in strippedLines if line.startswith('Total time:')]
	
	if len(times) != 1:
		if isExPNF:
			return mona_expnf_error 
		else:
			return mona_error
	time = parseTotalTime(times[0])
	
	# get all minimizings
	minimizations = [line for line in strippedLines if line.startswith('Minimizing')]	 
	automata_sizes = [int((re.search('\(([0-9]+),[0-9]+\)', min)).group(1)) for min in minimizations]
	output_size = sum(automata_sizes)
	
	# filter out half of the output till the crap
	index = 0
	while not strippedLines[index].startswith('Projecting'):
		index = index + 1
	strippedLines = strippedLines[index:]
	
	# get all minimizings
	minimizations = [line for line in strippedLines if line.startswith('Minimizing')]	 
	automata_sizes = [int((re.search('\(([0-9]+),[0-9]+\)', min)).group(1)) for min in minimizations]
	output_prefix_only_size = sum(automata_sizes)

	if isExPNF:
		return (time, output_size, output_prefix_only_size), ret
	else:
		return (time, output_size), ret

def parsedWiNAOutput(output, unprunedOutput, checkonly=False):
	'''
	
	@param output: lines with dwina output
	'''
	strippedLines = [line.lstrip() for line in output]
	ret = ""
	for line in strippedLines:
		match = re.search("\[!\] Formula is '([A-Z]+)'", line)
		if match is not None:
			ret = match.group(1)
			break

	if checkonly:
		return (-1, -1, -1, -1, -1), ret	

	# get total time
	times = [line for line in strippedLines if line.startswith('[*] Total elapsed time:')]
	if (len(times) != 1):
		return 
	time = parseTotalTime(times[0])
	
	# get dp time
	times = [line for line in strippedLines if line.startswith('[*] Decision procedure elapsed time:')]
	time_dp = parseTotalTime(times[0])
	
	# get size of state
	sizes = [line for line in strippedLines if line.startswith('[*] Number of states in resulting automaton:')]
	base_aut = int(re.search('[0-9]+', sizes[0]).group(0))
	
	# get size of state
	sizes = [line for line in strippedLines if line.startswith('[*] Size of the searched space:')]
	size = int(re.search('[0-9]+', sizes[0]).group(0))
	
	strippedLines = [line.lstrip() for line in unprunedOutput]
	sizes = [line for line in strippedLines if line.startswith('[*] Size of the searched space:')]
	size_unpruned = int(re.search('[0-9]+', sizes[0]).group(0))
	
	return (time, time_dp, base_aut, size, size_unpruned), ret

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

def generate_formulae(options, benchmark_name, up_to, alts, generator, zeroFill):
	for i in range(1, up_to + 1):
		if alts == 0:
			formula = generator(i)
		else:
			formula = generator(i, alts);
		if formula is not None:
			output_name = benchmark_name + str(i).zfill(zeroFill) + ("_{}alts".format(alts) if alts != 0 else "") + ".mona"
			output_path = os.path.join(options.dir, output_name)
			with open(output_path, 'w') as file:
				file.write(formula)

if __name__ == '__main__':
	print("[*] WSkS Test Bench")
	print("[c] Tomas Fiedor, ifiedortom@fit.vutbr.cz")
	
	options = parseArguments()
	
	# we will generate stuff
	if options.generate is not None or options.generate_alt is not None:
		generate_alternating = options.generate_alt is not None
		if not generate_alternating:
			print("[*] Generating benchmarks '{}' up to parameter n = {}".format(options.generate[0], options.generate[1]))
		else:
			print("[*] Generating benchmarks '{}' up to parameter n = {} with {} alternations".format(options.generate_alt[0], options.generate_alt[1], options.generate_alt[2]))
		benchmark_name = options.generate[0] if not generate_alternating else options.generate_alt[0]
		up_to = int(options.generate[1]) if not generate_alternating else int(options.generate_alt[1])
		alts = 0 if not generate_alternating else int(options.generate_alt[2])
		
		try:
			if generate_alternating:
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
		
	else:
		data = {}
		# modification and setup of parameters
		if options.bin is not None and len(options.bin) > 4:
			print("[!] Invalid number of binaries")
			quit()
		bins = ['mona', 'mona-expnf', 'dwina', 'dwina-dfa'] if (options.bin is None) else options.bin
		bins = ['mona', 'dwina'] if options.check else bins
		wdir = test_dir if options.check else options.dir
		
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
				print(colored("'{}'".format(benchmark), "grey", attrs=["bold"]))
				rets = {'dwina' : ""}
				for bin in bins:
					method_name = "_".join(["run"] + bin.split('-'))
					method_call = getattr(sys.modules[__name__], method_name)
					data[benchmark][bin], rets[bin] = method_call(benchmark, options.timeout, options.check)
					if rets['dwina'] == "" and 'dwina-dfa' in rets.keys():
						rets['dwina'] = rets['dwina-dfa']
				cases += 1
				all_cases.append("'{}': dWiNA ('{}') vs mona ('{}')".format(benchmark, rets['dwina'], rets['mona'].lower()))	
				if rets['mona'] == -1 or rets['mona'] == "":
					print("\t-> MONA failed or could not be determined")
				elif rets['mona'].upper() != rets['dwina']:
					print("\t->"),
					print(colored("FAIL", "red")),
					print("; Formula is "),
					print(colored("'{}'".format(rets['mona']), "white")),
					print(" (dWiNA returned "),
					print(colored("'{}'".format(rets['dwina'].lower()), "white")),
					print(")")
					fails += 1
					failed_cases.append("'{}': dWiNA ('{}') vs mona ('{}')".format(benchmark, rets['dwina'], rets['mona'].lower()))	
				else:
					print("\t->"),
					print(colored("OK","green")),
					print("; Formula is"),
					print(colored("'{}'".format(rets['mona']), "white"))
		if(options.check):
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
		else:
			if not options.no_export_to_csv:
				exportToCSV(data, bins)			   
