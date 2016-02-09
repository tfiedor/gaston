/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *      Symbolic Method of deciding the WS1S formulae. The method traverses
 *      the formulae symbolically and tests if initial states of automaton
 *      corresponding to the formulae and final states intersection is
 *      nonempty.
 *****************************************************************************/

#include "containers/SymbolicAutomata.h"
#include "containers/Term.h"
#include "decision_procedures.hh"
#include "environment.hh"

using namespace Gaston;