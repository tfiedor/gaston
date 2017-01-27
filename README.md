# Gaston - Decision Procedure for WS1S Logic
[![Build Status](https://travis-ci.org/tfiedor/gaston.svg?branch=master)](https://travis-ci.org/tfiedor/gaston)
## About 
  
  Gaston is an implementation of backward decision procedure for WS1S logic
  (weak second-order theory of k successors). The tool is using libmona a 
  highly optimised deterministic finite tree automata library that supports 
  semi-symbolic encoding using multi-terminal binary decision diagrams (MTBDDs) for
  storing transition table of the automaton. Procedure generates state space on-the-fly
  and prunes the states using the antichain-based approach. The tool works on the 
  symbolical representation of the formula as Symbolic Automata and tries to prove
  on-the-fly that the initial states intersect the final states to (dis)prove
  validity.
  
## Prerequisities 
 
 Following packages are needed in order to compile and run Gaston on your system:

 	git (>= 1.6.0.0)
 	cmake (>= 2.8.2)
 	gcc (>= 4.8.0)
 	flex (>= 2.5.35)
 	bison (>= 2.7.1)
 	
## Download
 
 In order to compile and work with our tool we highly recommend to clone the repository
 to the local repository. You need to have the git version control system installed.
 
 To download the library, run

``` 
   $ git clone https://github.com/tfiedor/Gaston.git
```
 	
## Compilation
 
 In order to prepare the tool for compilation and commiting a background check for
 prerequisite package issue the following command in the root directory of the tool:
 
 To compile the source files to runable binary issue the following command:
 
```
   $ make release
```
## Input file syntax

The following grammar describes supported subset of MONA syntax
for the specification of verified formulae. It uses the classical BNF-like
notation.

For full syntax for MONA tool consult the tool manual at http://www.brics.dk/mona/mona14.pdf.

```
program ::= (header;)? (declaration;)+
header ::=  ws1s | ws2s

declaration ::= formula
             |  var0 (varname)+
             |  var1 (varname)+
             |  var2 (varname)+
             |  'pred' varname (params)? = formula
             |  'macro' varname (params)? = formula

formula ::= 'true' | 'false' | (formula)
         |  zero-order-var
         | ~formula
         | formula | formula
         | formula & formula
         | formula => formula
         | formula <=> formula
         | first-order-term = first-order-term 
         | first-order-term ~= first-order-term 
         | first-order-term < first-order-term 
         | first-order-term > first-order-term
         | first-order-term <= first-order-term 
         | first-order-term >= first-order-term
         | second-order-term = second-order-term
         | second-order-term = { (int)+ }
         | second-order-term ~= second-order-term
         | second-order-term 'sub' second-order-term
         | first-order-term 'in' second-order-term
         | ex1 (varname)+ : formula
         | all1 (varname)+ : formula
         | ex2 (varname)+ : formula
         | all2 (varname)+ : formula

first-order-term ::= varname | (first-order-term)
                  |  int
                  | first-order-term + int

second-order-term ::= varname | (second-order-term)
                   |  second-order-term + int
```
 
## Contact 
 
 If you have any questions, do not hesitate to contact the tool/method authors:
 
  * Tomas Fiedor <ifiedortom@fit.vutbr.cz> (corresponding author of Gaston)
  * Lukas Holik <holik@fit.vutbr.cz>
  * Petr Janku <ijanku@fit.vutbr.cz> (optimizations of MONA wrapper)
  * Ondrej Lengal <ilengal@fit.vutbr.cz> (corresponding author of VATA)
  * Tomas Vojnar <vojnar@fit.vutbr.cz>

 ---
 Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 
