/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions:   Ondrej Lengal <ondra.lengal@gmail.com>
 *                              (author of VATA)
 *                          Petr Janku <ijanku@fit.vutbr.cz>
 *                              (MTBDD and automata optimizations)
 *
 *  Description:
 *      Representation of Zero Symbol used in decision procedure
 *****************************************************************************/

#include "Symbol.h"
#include <boost/functional/hash.hpp>
#include "../containers/SymbolicCache.hh"

extern VarToTrackMap varMap;

namespace Gaston {
    size_t hash_value(ZeroSymbol* s) {
#       if (OPT_SYMBOL_HASH_BY_APPROX == true)
        return boost::hash_value(s);
#       else
        if(s == nullptr) return 0;
#       if (OPT_SHUFFLE_HASHES == true)
        size_t seed = hash64shift(boost::hash_value(s->_trackMask.count()));
        boost::hash_combine(seed, hash64shift(boost::hash_value(s->_trackMask.find_first())));
        return hash64shift(seed);
#       else
        size_t seed = boost::hash_value(s->_trackMask.count());
        boost::hash_combine(seed, boost::hash_value(s->_trackMask.find_first()));
        return seed;
#       endif
#       endif
    }

    size_t hash_value_no_ptr(ZeroSymbol* s) {
        if(!s->hash) {
#       if (OPT_SHUFFLE_HASHES == true)
            size_t seed = hash64shift(boost::hash_value(s->_trackMask.count()));
            boost::hash_combine(seed, hash64shift(boost::hash_value(s->_trackMask.find_first())));
            s->hash = hash64shift(seed);
#       else
            size_t seed = boost::hash_value(s->_trackMask.count());
            boost::hash_combine(seed, boost::hash_value(s->_trackMask.find_first()));
            s->hash = seed;
#       endif
        }
        return s->hash;
    }
}

size_t ZeroSymbol::instances = 0;

// <<< CONSTRUCTORS >>>
/**
 * Constructor that creates a new zero symbol
 */
ZeroSymbol::ZeroSymbol() : _trackMask(varMap.TrackLength() << 1) {
#   if (MEASURE_SYMBOLS == true)
    ++ZeroSymbol::instances;
#   endif
}

/**
 * Construct that creates a new symbol of @p track
 *
 * @param[in] track:    track of the constructed symbol
 */
ZeroSymbol::ZeroSymbol(BitMask const& track) {
#   if (MEASURE_SYMBOLS == true)
    ++ZeroSymbol::instances;
#   endif
    this->_trackMask = track;
}

/**
 * Constructor that creates a new symbol of @p track and sets the value of
 * @p var to @p val.
 *
 * @param[in] track:    track of the symbol
 * @param[in] var:      variable that is set to certain value @val
 * @param[in] val:      value of @p var
 */
ZeroSymbol::ZeroSymbol(BitMask const& track, VarType var, VarValue val) {
#   if (MEASURE_SYMBOLS == true)
    ++ZeroSymbol::instances;
#   endif
    this->_trackMask = track;
    this->_SetValueAt(var, ZeroSymbol::charToAsgn(val));
}

ZeroSymbol::ZeroSymbol(ZeroSymbol* src, std::map<unsigned int, unsigned int>* map)  : _trackMask(varMap.TrackLength() << 1) {
    // Symbol = XXXXXX
    this->_trackMask.set();
    for(auto it = map->begin(); it != map->end(); ++it) {
        unsigned int from = it->first;
        unsigned int to = it->second;
        this->_SetValueAt(to, src->GetSymbolAt(from));
    }
}

ZeroSymbol::~ZeroSymbol() {
    this->_trackMask.clear();
}

// <<< PRIVATE METHODS >>>
void ZeroSymbol::_SetDontCareAt(VarType var) {
    this->_trackMask.set(2*var, true);
    this->_trackMask.set(2*var+1, true);
}

void ZeroSymbol::_SetZeroAt(VarType var) {
    this->_trackMask.set(2*var, false);
    this->_trackMask.set(2*var+1, false);
}

void ZeroSymbol::_SetOneAt(VarType var) {
    this->_trackMask.set(2*var, true);
    this->_trackMask.set(2*var+1, false);
}

bool ZeroSymbol::IsDontCareAt(VarType var) {
    return this->_trackMask.test(2*var+1);
}

bool ZeroSymbol::IsZeroString() const {
    if(this->_trackMask.none()) {
        return true;
    } else {
        for(unsigned int i = 0; i < this->_trackMask.size(); i += 2) {
            if(this->_trackMask.test(i) && !this->_trackMask.test(i+1)) {
                return false;
            }
        }
        return true;
    }
}

void ZeroSymbol::_SetValueAt(VarType var, VarValue val) {
    if(val > 0x03) {
        val = charToAsgn(val);
    }
    switch(val) {
        case 0x01:
            this->_SetZeroAt(var);
            break;
        case 0x02:
            this->_SetOneAt(var);
            break;
        case 0x03:
            this->_SetDontCareAt(var);
            break;
        default:
            assert(false);
            break;   // fail gracefully
    }
}

std::string ZeroSymbol::ToString() const {
    std::string s("");
    for(unsigned int i = 0; i < this->_trackMask.size()/2; ++i) {
        s += this->GetSymbolAt(i);
    }

    return s;
}

char ZeroSymbol::GetSymbolAt(size_t pos) const {
    assert(pos < this->_trackMask.size()+1);

    bool lTest = this->_trackMask.test(2*pos);
    bool rTest = this->_trackMask.test(2*pos+1);

    if(lTest && !rTest) {
        return '1';
    } else if(!lTest && !rTest) {
        return '0';
    } else if(lTest) {
        return 'X';
    } else {
        std::cerr << "[!] Warning strange character in symbol occured\n";
        return '?';
    }
}

// <<< STATIC FUNCTIONS >>>

/**
 * Converts char @p c to representation of TrackType
 *
 * @param[in] c:    character to be converted
 */
char ZeroSymbol::charToAsgn(char c) {
    switch(c) {
        case '0':
            return 0x01;
        case '1':
            return 0x02;
        case 'X':
            return 0x03;
        default:
            assert(false);
            return 0x0; // unreachable dead code, only to remove warning
    }
}

/**
 * Static member that constructs the universal track X*
 */
TrackType ZeroSymbol::constructUniversalTrack() {
    assert(false);
    unsigned int trackLen = varMap.TrackLength() - 1;
    Automaton::SymbolType transitionTrack;
    transitionTrack.AddVariablesUpTo(trackLen);
    return transitionTrack;
}

// <<< PUBLIC API >>>
/**
 * Project the variable away, by setting it to don't care X
 *
 * @param[in] var:  track index of variable that is projected away
 */
void ZeroSymbol::ProjectVar(VarType var) {
    this->_SetDontCareAt(var);
}

// <<< FRIEND FUNCTIONS >>>
/**
 * Prints ZeroSymbol to output stream
 *
 * @param[in] osObject:     output stream
 * @param[in] z:            printed zero symbol
 */
std::ostream& operator <<(std::ostream& osObject, const ZeroSymbol& z) {
    osObject << z.ToString();
    return osObject;
}