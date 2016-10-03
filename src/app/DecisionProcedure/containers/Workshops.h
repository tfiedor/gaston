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
 *      Note: Workshop additionaly generates unique Empty symbol (but is it needed)
 *      Note#2: Workshops should be created on the node level, so we do not have
 *      one global workshop, but lots of smaller workshops with lesser items. While
 *      this seems to be inefficient in terms of space, in terms of speed and
 *      cache efficiency it is to my knowledge the best approach.
 *****************************************************************************/

#ifndef WSKS_WORKSHOPS_H
#define WSKS_WORKSHOPS_H


#include <unordered_map>
#include <functional>
#include <tuple>
#include <boost/functional/hash.hpp>
#include <boost/pool/object_pool.hpp>
#include "SymbolicCache.hh"
#include "../environment.hh"
#include "../../Frontend/ident.h"

// <<< FORWARD DECLARATION >>>
class TermEmpty;
class TermBaseSet;
class TermProduct;
class TermTernaryProduct;
class TermNaryProduct;
class TermFixpoint;
class TermList;
class TermContinuation;
class SymbolicAutomaton;
struct SymLink;
struct IntersectNonEmptyParams;

namespace Workshops {
    struct ComputationHash;
    struct ComputationCompare;

    using SymbolList        = Gaston::SymbolList;
    using Symbol            = Gaston::Symbol;
    using Symbol_ptr        = Symbol*;
    using VarType           = Gaston::VarType;
    using ValType           = Gaston::VarValue;

    using CacheData         = Term*;
    using SymbolKey         = std::tuple<Symbol*, VarType, ValType>;
    using SymbolHash        = boost::hash<SymbolKey>;
    using SimpleSymbolComp  = std::equal_to<Symbol*>;
    using SymbolCompare     = std::equal_to<SymbolKey>;
    using BaseKey           = VATA::Util::OrdVector<size_t>;
    using BaseHash          = boost::hash<BaseKey>;
    using BaseCompare       = std::equal_to<BaseKey>;
    using ProductKey        = std::pair<Term*, Term*>;
    using ProductHash       = boost::hash<ProductKey>;
    using ProductCompare    = PairCompare<ProductKey>;
    using TernaryKey        = std::tuple<Term*, Term*, Term*>;
    using TernaryHash       = boost::hash<TernaryKey>;
    using TernaryCompare    = std::equal_to<TernaryKey>;
    using NaryKey           = std::pair<Term**, size_t>;
    using ListKey           = Term*;
    using ListHash          = boost::hash<ListKey>;
    using ListCompare       = std::equal_to<ListKey>;
    using FixpointKey       = std::pair<Term*, Symbol*>;
    using FixpointHash      = boost::hash<FixpointKey>;
    using FixpointCompare   = PairCompare<FixpointKey>;
    using FixpointLevelKey  = std::tuple<Term*, size_t, char>;
    using FixpointLevelHash = boost::hash<FixpointLevelKey>;
    using FixpointLevelComp = std::equal_to<FixpointLevelKey>;
    using Term_ptr          = Term*;
    using FixpointType      = std::list<std::pair<Term*,bool>>;
    using WorklistItemType  = std::pair<Term*, Symbol*>;
    using WorklistType      = std::list<WorklistItemType>;
    using ComputationKey    = std::pair<size_t, TermFixpoint*>;
    using RemapKey          = std::pair<Symbol*, size_t>;

    void dumpBaseKey(BaseKey const&);
    void dumpProductKey(ProductKey const&);
    void dumpTernaryKey(TernaryKey const&);
    void dumpListKey(ListKey const&);
    void dumpFixpointKey(FixpointKey const&);
    void dumpFixpointLevelKey(FixpointLevelKey const&);
    void dumpComputationKey(ComputationKey const&);
    void dumpCacheData(CacheData &);
    void dumpSymbolKey(SymbolKey const&);
    void dumpSymbolData(Symbol* &);
    void dumpNaryKey(NaryKey const&);
    template<class Key>
    void dumpDefaultKey(Key const&);
    template<class Data>
    void dumpDefaultData(Data&);
    void dumpRemapKey(RemapKey const&);

    struct SimpleSymbolHash {
        size_t operator()(Symbol* const& s) const {
            return Gaston::hash_value_no_ptr(s);
        }
    };

    struct SymbolKeyHashType {
        size_t operator()(std::tuple<Symbol*, VarType, ValType> const& set) const {
            size_t seed = (Gaston::hash_value_no_ptr(std::get<0>(set)));
            boost::hash_combine(seed, hash64shift(boost::hash_value(std::get<1>(set))));
            boost::hash_combine(seed, hash64shift(boost::hash_value(std::get<2>(set))));
            return seed;
        }
    };

    struct RemappedSymbolHash {
        size_t operator()(std::pair<Symbol*, size_t> const& s) const {
            size_t seed = Gaston::hash_value_no_ptr(s.first);
            boost::hash_combine(seed, boost::hash_value(s.second));
            return seed;
        }
    };

    struct RemappedKeyCompare : public std::binary_function<RemapKey, RemapKey, bool> {
        bool operator()(RemapKey const& lhs, RemapKey const& rhs) const {
            return lhs.first == rhs.first && lhs.second == rhs.second;
        }
    };

    template<class Key>
    struct SymbolKeyCompare : public std::binary_function<Key, Key, bool> {
        bool operator()(Key const& lhs, Key const& rhs) const {
            return std::get<1>(lhs) == std::get<1>(rhs) && std::get<2>(lhs) == std::get<2>(rhs) &&
                    *(std::get<0>(lhs)) == *(std::get<0>(rhs));
        }
    };

    struct TernaryKeyHashType {
        size_t operator()(std::tuple<Term*, Term*, Term*> const& set) const {
            size_t seed = Gaston::hash_value(std::get<0>(set));
            boost::hash_combine(seed, Gaston::hash_value(std::get<1>(set)));
            boost::hash_combine(seed, Gaston::hash_value(std::get<2>(set)));
            return seed;
        }
    };

    template<class Key>
    struct TernaryKeyCompare : public std::binary_function<Key, Key, bool> {
        bool operator()(Key const& lhs, Key const& rhs) const {
            return *(std::get<0>(lhs)) == *(std::get<0>(rhs)) &&
                   *(std::get<1>(lhs)) == *(std::get<1>(rhs)) &&
                   *(std::get<2>(lhs)) == *(std::get<2>(rhs));
        }
    };

    struct NaryKeyHashType {
        size_t operator()(std::pair<Term_ptr*, size_t> const& set) const {
            size_t seed = 0;
            for(auto i = 0; i < set.second; ++i) {
                boost::hash_combine(seed, Gaston::hash_value(set.first[i]));
            }
            return seed;
        }
    };

    template<class Key>
    struct NaryKeyCompare : public std::binary_function<Key, Key, bool> {
        bool operator()(Key const& lhs, Key const& rhs) const {
            assert(lhs.second == rhs.second);
            for(auto i = 0; i < lhs.second; ++i) {
                if(*lhs.first != *rhs.first) {
                    return false;
                }
            }
            return true;
        }
    };

    using SymbolCache       = BinaryCache<SymbolKey, Symbol*, SymbolKeyHashType, SymbolKeyCompare<SymbolKey>, dumpSymbolKey, dumpSymbolData>;
    using SimpleSymbolCache = BinaryCache<Symbol_ptr, Symbol_ptr, SimpleSymbolHash, SimpleSymbolComp, dumpDefaultKey, dumpDefaultData>;
    using RemapCache        = BinaryCache<RemapKey, Symbol_ptr, RemappedSymbolHash, RemappedKeyCompare, dumpRemapKey, dumpSymbolData>;
    using BaseCache         = BinaryCache<BaseKey, CacheData, BaseHash, BaseCompare, dumpBaseKey, dumpCacheData>;
    using ProductCache      = BinaryCache<ProductKey, CacheData, ProductHash, ProductCompare, dumpProductKey, dumpCacheData>;
    using TernaryCache      = BinaryCache<TernaryKey, CacheData, TernaryKeyHashType, TernaryKeyCompare<TernaryKey>, dumpTernaryKey, dumpCacheData>;
    using ListCache         = BinaryCache<ListKey, CacheData, ListHash, ListCompare, dumpListKey, dumpCacheData>;
    using FixpointCache     = BinaryCache<FixpointKey, CacheData, FixpointHash, FixpointCompare, dumpFixpointKey, dumpCacheData>;
    using FixpointLevelCache= BinaryCache<FixpointLevelKey, CacheData, FixpointLevelHash, FixpointLevelComp, dumpFixpointLevelKey, dumpCacheData>;
    using ComputationCache  = BinaryCache<ComputationKey, CacheData, ComputationHash, ComputationCompare, dumpComputationKey, dumpCacheData>;
    using NaryCache         = BinaryCache<NaryKey, CacheData, NaryKeyHashType, NaryKeyCompare<NaryKey>, dumpNaryKey, dumpCacheData>;

    class TermWorkshop {
    private:
        // <<< PRIVATE MEMBERS >>>
        BaseCache* _bCache;
        ProductCache* _ubCache; // Union of Bases Cache
        ProductCache* _pCache;
        TernaryCache* _tpCache;
        NaryCache* _npCache;
        ListCache* _lCache;
        ListCache* _fpCache;
        FixpointCache* _fppCache;
        FixpointCache* _contCache;
        FixpointLevelCache* _fplCache;
        ComputationCache* _compCache;

        static TermEmpty *_empty;
        static TermEmpty *_emptyComplement;

        SymbolicAutomaton* _aut; // ProjectionAutomaton for Fixpoints

#       if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
        static boost::object_pool<TermFixpoint> _fixpointPool;
        static boost::object_pool<TermProduct> _productPool;
        static boost::object_pool<TermTernaryProduct> _ternaryProductPool;
        static boost::object_pool<TermNaryProduct> _naryProductPool;
        static boost::object_pool<TermBaseSet> _basePool;
#       endif
    private:
        // <<< PRIVATE FUNCTIONS >>>
        template<class A, class B, class C, class D, void (*E)(const A&), void (*F)(B&)>
        inline static BinaryCache<A, B, C, D, E, F>* _cleanCache(BinaryCache<A, B, C, D, E, F>*, bool noMemberDelete = false);

    public:
        static unsigned long monaAutomataStates;

        // <<< CONSTRUCTORS >>>
        explicit TermWorkshop(SymbolicAutomaton*);
        ~TermWorkshop();
        void InitializeWorkshop();

        // <<< PUBLIC API >>>
        static TermEmpty* CreateEmpty();
        static TermEmpty* CreateComplementedEmpty();
        Term* CreateBaseSet(BaseKey && states);
        Term* CreateUnionBaseSet(Term_ptr const&, Term_ptr const&);
        TermProduct* CreateProduct(Term_ptr const&, Term_ptr const&, ProductType);
        Term* CreateTernaryProduct(Term_ptr const&, Term_ptr const&, Term_ptr const&, ProductType);
        Term* CreateNaryProduct(Term_ptr const&, Symbol*, size_t, ProductType);
        Term* CreateNaryProduct(Term_ptr* const&, size_t, ProductType);
        Term* CreateBaseNaryProduct(SymLink*, size_t, StatesSetType, ProductType);
        Term* CreateUniqueNaryProduct(Term_ptr* const&, size_t, ProductType);
        TermFixpoint* CreateFixpoint(Term_ptr const&, Symbol*, bool, bool, WorklistSearchType search = WorklistSearchType::DFS);
        TermFixpoint* CreateFixpointPre(Term_ptr const&, Symbol*, bool);
        TermFixpoint* CreateFixpointPreLevel(Term_ptr const&, Symbol*, size_t, char, bool);
        TermFixpoint* CreateClonedFixpoint(TermFixpoint* const&, IntersectNonEmptyParams&);
        TermFixpoint* GetUniqueFixpoint(TermFixpoint*&, size_t);
        Term* CreateList(Term_ptr const&, bool);
        Term* CreateContinuation(SymLink*, SymbolicAutomaton*, Term* const&, Symbol*, bool, bool lazy = false);
        std::string ToSimpleStats();

        void Dump();
    };

    class SymbolWorkshop {
    private:
        SymbolCache* _symbolCache = nullptr;
        SimpleSymbolCache* _trimmedSymbolCache = nullptr;
        RemapCache* _remappedSymbolCache = nullptr;
        std::list<Symbol*> _trimmedSymbols;
        std::list<Symbol*> _remappedSymbols;
        Symbol* _CreateProjectedSymbol(Symbol*, VarType, ValType);

#       if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
        static boost::object_pool<Symbol> _pool;
#       endif
    public:
        static Symbol* _zeroSymbol;

        // <<< CONSTRUCTORS >>>
        SymbolWorkshop();
        ~SymbolWorkshop();

        static Symbol* CreateZeroSymbol();
        Symbol* CreateSymbol(Symbol*, VarType, ValType);
        Symbol* CreateTrimmedSymbol(Symbol*, Gaston::VarList*);
        Symbol* CreateRemappedSymbol(Symbol*, std::map<unsigned int, unsigned int>*&, size_t);

        void Dump();
    };
}

#endif //WSKS_WORKSHOPS_H
