/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of common Automata related operations
 *
 *****************************************************************************/

#include "automata.hh"

#include <cstdio>
#include <cstdlib>

/**
 * Converts character to specification of VATA
 *
 * @param[in] c: classical character from {0, 1, X}
 * @return: character from enum {ONE, ZERO, DONT_CARE} from VATA spec.
 */
char charToAsgn(char c) {
	switch(c) {
	case '0':
		return 0x01;
	case '1':
		return 0x02;
	case 'X':
		return 0x03;
	default:
		assert(false);
		abort();
	}
}
