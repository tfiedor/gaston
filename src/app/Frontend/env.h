/*
 * MONA
 * Copyright (C) 1997-2013 Aarhus University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the  Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335,
 * USA.
 */

#ifndef __ENV_H
#define __ENV_H

enum Mode {LINEAR, TREE};
enum ReorderMode {NO, RANDOM, HEURISTIC};
enum Method {FORWARD, BACKWARD, SYMBOLIC};
enum AutomataConstruction {DETERMINISTIC_AUT, NONDETERMINISTIC_AUT, SYMBOLIC_AUT};
enum TestType {EVERYTHING, VALIDITY, SATISFIABILITY, UNSATISFIABILITY};

class Options {
public:
  Options() :
    optimize(0),
    fixLimit(0),
    inverseFixLimit(-1),

    method(SYMBOLIC),
    construction(AutomataConstruction::SYMBOLIC_AUT),
    mode(LINEAR),
    test(EVERYTHING),
    reorder(HEURISTIC),

    noExpnf(true),
    time(false),
    whole(false),
    statistics(false),
    printProgress(false),
    analysis(false),
    separateCompilation(false),
    dump(false),
    intermediate(false),
    treemodeOutput(false),
    m2l(false),
    graphvizDFA(false),
    graphvizDAG(false),
    graphvizSatisfyingEx(false),
    graphvizCounterEx(false),
    externalWhole(false),
    demo(false),
    inheritedAcceptance(false),
    unrestrict(false),
    alternativeM2LStr(false),
    monaWalk(false),
    expandTagged(false),
    useMonaDFA(false),
    serializeMona(false),
    dryRun(false),
    verifyModels(false) {}


  unsigned optimize;
  int fixLimit;
  int inverseFixLimit;

  Method method;
  AutomataConstruction construction;
  Mode mode;
  TestType test;
  ReorderMode reorder;

  bool noExpnf;
  bool time;
  bool whole;
  bool statistics;
  bool printProgress;
  bool analysis;
  bool separateCompilation;
  bool dump;
  bool dontDumpAutomaton;
  bool intermediate;
  bool treemodeOutput;
  bool m2l;
  bool graphvizDFA;
  bool graphvizDAG;
  bool graphvizSatisfyingEx;
  bool graphvizCounterEx;
  bool externalWhole;
  bool demo;
  bool inheritedAcceptance;
  bool unrestrict;
  bool alternativeM2LStr;
  bool monaWalk;
  bool expandTagged;
  bool useMonaDFA;
  bool serializeMona;
  bool dryRun;
  bool verifyModels;
};

#endif
