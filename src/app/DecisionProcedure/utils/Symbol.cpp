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

extern VarToTrackMap varMap;

namespace Gaston {
    size_t hash_value(ZeroSymbol* s) {
        if(s == nullptr) return 0;
        size_t seed = boost::hash_value(s->_trackMask.count());
        boost::hash_combine(seed, boost::hash_value(s->_trackMask.find_first()));
        return seed;
    }
}

size_t ZeroSymbol::instances = 0;

// <<< CONSTRUCTORS >>>
/**
 * Constructor that creates a new zero symbol
 */
ZeroSymbol::ZeroSymbol() : _trackMask(varMap.TrackLength() << 1) {
    #if (MEASURE_SYMBOLS == true)
    ++ZeroSymbol::instances;
    #endif
    this->_bdd = nullptr;
}

/**
 * Construct that creates a new symbol of @p track
 *
 * @param[in] track:    track of the constructed symbol
 */
ZeroSymbol::ZeroSymbol(BitMask const& track) {
    #if (MEASURE_SYMBOLS == true)
    ++ZeroSymbol::instances;
    #endif
    this->_trackMask = track;
    this->_bdd = nullptr;
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
    #if (MEASURE_SYMBOLS == true)
    ++ZeroSymbol::instances;
    #endif
    this->_trackMask = track;
    this->_SetValueAt(var, ZeroSymbol::charToAsgn(val));
    this->_bdd = nullptr;
}

ZeroSymbol::~ZeroSymbol() {
    if(this->_bdd != nullptr) {
        delete this->_bdd;
        this->_bdd = nullptr;
    }
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
    return this->_trackMask.test(2*var) && this->_trackMask.test(2*var+1);
}

void ZeroSymbol::_SetValueAt(VarType var, VarValue val) {
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
    for(int i = 0; i < this->_trackMask.size(); i = i + 2) {
        bool lTest = this->_trackMask.test(i);
        bool rTest = this->_trackMask.test(i+1);
        if(lTest && !rTest) {
            s += "1";
        } else if(!lTest && !rTest) {
            s += "0";
        } else {
            s += "X";
        }
    }

    return s;
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
            break;
        case '1':
            return 0x02;
            break;
        case 'X':
            return 0x03;
            break;
        default:
            assert(false);
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

/**
 * Static member, that constructs the zero track 0*
 */
TrackType ZeroSymbol::constructZeroTrack() {
    unsigned int trackLen = varMap.TrackLength();
    std::string track(trackLen, '0');
    return TrackType(track.c_str());
}

// <<< PUBLIC API >>>
/**
 * Returns the MTBDD. If it is not initialized, creates a new MTBDD and return it.
 */
BaseAutomatonMTBDD* ZeroSymbol::GetMTBDD() {
    if(this->_bdd == nullptr) {
        this->_bdd = new BaseAutomatonMTBDD(TrackType(this->ToString()), BaseAutomatonStateSet(StateTuple({0})), BaseAutomatonStateSet(StateTuple({})));
    }

    // Initialization was successful
    assert(this->_bdd != nullptr && "MTBDD for base automaton was not initialized\n");
    return this->_bdd;
}

/**
 * Project the variable away, by setting it to don't care X
 *
 * @param[in] var:  track index of variable that is projected away
 */
void ZeroSymbol::ProjectVar(VarType var) {
    assert(this->_bdd == nullptr);
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