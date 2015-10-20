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

#ifndef WSKS_SYMBOL_H
#define WSKS_SYMBOL_H

#include "../containers/VarToTrackMap.hh"
#include "../environment.hh"

using namespace Gaston;

/**
 * Class that represents one symbol on track
 */
class ZeroSymbol {
private:
    // <<< PRIVATE MEMBERS >>>
    TrackType _track;
    BaseAutomatonMTBDD* _bdd;

public:
    // <<< CONSTRUCTORS >>>
    ZeroSymbol();
    ZeroSymbol(TrackType);
    ZeroSymbol(TrackType, VarType, VarValue);

    // <<< PUBLIC API >>>
    void ProjectVar(VarType var);
    TrackType GetTrack() { return this->_track; }
    BaseAutomatonMTBDD* GetMTBDD();

    // <<< STATIC METHODS >>>
    static TrackType constructUniversalTrack();
    static TrackType constructZeroTrack();
    static char charToAsgn(char c);

    // <<< FRIENDS >>>
    friend std::ostream& operator <<(std::ostream& osObject, const ZeroSymbol& z);
    friend bool operator==(const std::shared_ptr<ZeroSymbol>& lhs, const std::shared_ptr<ZeroSymbol>& rhs);
};

/**
 * Does the comparison of two Zero Symbols
 *
 * @param[in] lhs:      left operand
 * @param[in] rhs:      right operand
 */
inline bool operator==(const std::shared_ptr<ZeroSymbol>& lhs, const std::shared_ptr<ZeroSymbol>& rhs) {
    if(lhs == nullptr || rhs == nullptr) {
        return lhs == nullptr && rhs == nullptr;
    } else if(lhs.get() == rhs.get()) {
        return true;
    } else {
        return lhs->_track.ToString() == rhs->_track.ToString();
    }
}

#endif //WSKS_SYMBOL_H
