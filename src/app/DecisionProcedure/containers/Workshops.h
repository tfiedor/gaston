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
 *      Workshops is bundle of classes that encapsulates the creation of the
 *      objects. Appart of handling the creation the main focus of workshops
 *      is to generate unique pointers for the same objects. This should
 *      help with (a) caching, (b) storing of the objects and holding them
 *      in the memory and (c) equality testing.
 *
 *  TermWorkshop:
 *      Workshop for generation of the terms. Can generate the following terms
 *      and additionaly caches according to the mentioned keys:
 *        1) TermBaseSet - with caching
 *          VATA::Util::OrdVector -> TermBaseSet*
 *        2) TermProduct - with caching ?
 *          <Term*, Term*> -> TermProduct*
 *        3) TermContinuation - with caching ?
 *          <Term*, Symbol*> -> TermContinuation*
 *        4) TermList - with caching ?
 *          vector<Term*> -> TermList*
 *        5) TermFixpoint - no caching ?
 *        TODO: how this should be handled?
 *      Note: Workshop additionaly generates unique Empty symbol (but is it needed)
 *      Note#2: Workshops should be created on the node level, so we do not have
 *      one global workshop, but lots of smaller workshops with lesser items. While
 *      this seems to be inefficient in terms of space, in terms of speed and
 *      cache efficiency it is to my knowledge the best approach.
 *****************************************************************************/

#ifndef WSKS_WORKSHOPS_H
#define WSKS_WORKSHOPS_H


#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <functional>
#include "../environment.hh"
#include "SymbolicCache.hh"

class TermBaseSet;

// TODO: 1) Dump it out
// TODO: 2) Cache it out
// TODO: 3) More shops

namespace Workshops {
    // TODO: Maybe I forgot to take measure of the complement?
    void dumpBaseKey(VATA::Util::OrdVector<unsigned int> const&);
    void dumpCacheData(Term *&);

    using CacheData     = Term *;
    using BaseKey       = VATA::Util::OrdVector<unsigned int>;
    using BaseHash      = boost::hash<BaseKey>;
    using BaseCompare   = std::equal_to<BaseKey>;
    using BaseCache     = BinaryCache<BaseKey, CacheData, BaseHash, BaseCompare, dumpBaseKey, dumpCacheData>;

    class TermWorkshop {
        //using SubsumptionCache       = BinaryCache<SubsumptionKey, bool, SubsumptionHashType, PairCompare<SubsumptionKey>, dumpSubsumptionKey, dumpSubsumptionData>;
    private:
        // <<< PRIVATE MEMBERS >>>
        BaseCache* _bCache;
    public:
        // <<< CONSTRUCTORS >>>
        TermWorkshop();

        // <<< PUBLIC API >>>
        TermBaseSet *CreateBaseSet(BaseKey &states, unsigned int offset, unsigned int stateno);

        void Dump();
    };
}

#endif //WSKS_WORKSHOPS_H
