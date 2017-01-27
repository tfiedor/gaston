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

namespace Gaston {
    size_t hash_value(ZeroSymbol*);
    size_t hash_value_no_ptr(ZeroSymbol*);
}

/**
 * Class that represents one symbol on track
 */
class ZeroSymbol {
protected:
    // <<< PRIVATE MEMBERS >>>
    BitMask _trackMask;

    // <<< PRIVATE METHODS >>>
    inline void _SetZeroAt(VarType var);
    inline void _SetOneAt(VarType var);
    inline void _SetDontCareAt(VarType var);
    inline void _SetValueAt(VarType var, VarValue val);

public:
    // <<< STATIC MEMBERS >>>
    static size_t instances;
    size_t hash = 0;

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE ZeroSymbol();
    NEVER_INLINE ZeroSymbol(ZeroSymbol*, std::map<unsigned int, unsigned int>*);
    NEVER_INLINE explicit ZeroSymbol(BitMask const&);
    NEVER_INLINE ZeroSymbol(BitMask const&, VarType, VarValue);
    NEVER_INLINE ~ZeroSymbol();

    // <<< PUBLIC API >>>
    void ProjectVar(VarType);
    BitMask GetTrackMask() { return this->_trackMask; }
    bool IsDontCareAt(VarType);
    BaseAutomatonMTBDD* GetMTBDD();
    std::string ToString() const;
    char GetSymbolAt(size_t) const;
    bool IsZeroString() const;

    // <<< STATIC METHODS >>>
    static char charToAsgn(char c);

    // <<< FRIENDS >>>
    friend std::ostream& operator <<(std::ostream& osObject, const ZeroSymbol& z);
    friend bool operator==(const ZeroSymbol& lhs, const ZeroSymbol& rhs);

    friend size_t Gaston::hash_value(ZeroSymbol* s);
    friend size_t Gaston::hash_value_no_ptr(ZeroSymbol* s);
};

inline bool operator==(const ZeroSymbol& lhs, const ZeroSymbol& rhs) {
    return &lhs == &rhs || lhs._trackMask == rhs._trackMask;
}

#endif //WSKS_SYMBOL_H