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
 *      Representation of Zero Symbol used in decision procedure
 *****************************************************************************/

#include "Symbol.h"

extern VarToTrackMap varMap;

// <<< CONSTRUCTORS >>>
/**
 * Constructor that creates a new zero symbol
 */
ZeroSymbol::ZeroSymbol() : _trackMask(varMap.TrackLength() << 1) {
    this->_track = ZeroSymbol::constructZeroTrack();
    this->_bdd = nullptr;
}

/**
 * Construct that creates a new symbol of @p track
 *
 * @param[in] track:    track of the constructed symbol
 */
ZeroSymbol::ZeroSymbol(TrackType track) : _trackMask(varMap.TrackLength() << 1) {
    this->_track = track;
    this->_bdd = nullptr;

    this->_InitializeTrackMask();
}

/**
 * Constructor that creates a new symbol of @p track and sets the value of
 * @p var to @p val.
 *
 * @param[in] track:    track of the symbol
 * @param[in] var:      variable that is set to certain value @val
 * @param[in] val:      value of @p var
 */
ZeroSymbol::ZeroSymbol(TrackType track, VarType var, VarValue val) : _trackMask(varMap.TrackLength() << 1) {
    this->_track = track;
    if(this->_track.GetIthVariableValue(var) != charToAsgn('X')) {
        this->_track.SetIthVariableValue(var, charToAsgn(val));
    }
    this->_bdd = nullptr;

    this->_InitializeTrackMask();
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

void ZeroSymbol::_InitializeTrackMask() {
    for (auto i = 0; i < this->_track.length(); ++i)
    {	// append all variables to the string
        switch (this->_track.GetIthVariableValue(i))
        {
            case 0x01:
                this->_SetZeroAt(i);
                break;
            case 0x02:
                this->_SetOneAt(i);
                break;
            case 0x03:
                this->_SetDontCareAt(i);
                break;
            default:
                assert(false);
                break;   // fail gracefully
        }
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
        this->_bdd = new BaseAutomatonMTBDD(this->_track, BaseAutomatonStateSet(StateTuple({0})), BaseAutomatonStateSet(StateTuple({})));
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

    this->_track.SetIthVariableValue(var, charToAsgn('X'));
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
    osObject << z._track.ToString();
    return osObject;
}

namespace Gaston {
    size_t hash_value(std::shared_ptr <ZeroSymbol> &s) {
        if(s == nullptr) {
            return 0;
        } else {
            return s->_trackMask.count();
        }
    }

    size_t hash_value(const ZeroSymbol &s) {
        return s._trackMask.count();
    }
}