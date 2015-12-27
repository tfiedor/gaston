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
protected:
    // <<< PRIVATE MEMBERS >>>
    BaseAutomatonMTBDD* _bdd;
    BitMask _trackMask;

    // <<< PRIVATE METHODS >>>
    inline void _SetZeroAt(VarType var);
    inline void _SetOneAt(VarType var);
    inline void _SetDontCareAt(VarType var);
    inline void _SetValueAt(VarType var, VarValue val);

public:
    // <<< CONSTRUCTORS >>>
    ZeroSymbol();
    ZeroSymbol(BitMask);
    ZeroSymbol(BitMask, VarType, VarValue);

    // <<< PUBLIC API >>>
    void ProjectVar(VarType var);
    BitMask GetTrackMask() { return this->_trackMask; }
    BaseAutomatonMTBDD* GetMTBDD();
    std::string ToString() const;

    // <<< STATIC METHODS >>>
    static TrackType constructUniversalTrack();
    static TrackType constructZeroTrack();
    static char charToAsgn(char c);

    // <<< FRIENDS >>>
    friend std::ostream& operator <<(std::ostream& osObject, const ZeroSymbol& z);
    friend bool operator==(const std::shared_ptr<ZeroSymbol>& lhs, const std::shared_ptr<ZeroSymbol>& rhs);
    friend bool operator==(const ZeroSymbol& lhs, const ZeroSymbol& rhs);
    friend size_t Gaston::hash_value(std::shared_ptr <ZeroSymbol> &s){
        if(s == nullptr) {
            return 0;
        } else {
            return s->_trackMask.count();
        }
    }
    friend size_t Gaston::hash_value(const ZeroSymbol& s){
        size_t seed = 0;
        boost::hash_combine(seed, boost::hash_value(s._trackMask.count()));
        boost::hash_combine(seed, boost::hash_value(s._trackMask.find_first()));
        return seed;
    }
    friend size_t Gaston::hash_value(ZeroSymbol* s){
        size_t seed = 0;
        boost::hash_combine(seed, boost::hash_value(s->_trackMask.count()));
        boost::hash_combine(seed, boost::hash_value(s->_trackMask.find_first()));
        return seed;
    }
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
        return lhs->_trackMask == rhs->_trackMask;
    }
}

inline bool operator==(const ZeroSymbol& lhs, const ZeroSymbol& rhs) {
    return lhs._trackMask == rhs._trackMask;
}

#endif //WSKS_SYMBOL_H