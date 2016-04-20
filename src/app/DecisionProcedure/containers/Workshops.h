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


#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <functional>
#include <tuple>
#include "../environment.hh"
#include "../../Frontend/ident.h"
#include "SymbolicCache.hh"

// <<< FORWARD DECLARATION >>>
class TermEmpty;
class TermBaseSet;
class TermProduct;
class TermFixpoint;
class TermList;
class TermContinuation;
class SymbolicAutomaton;

namespace Workshops {
    struct ComputationHash;
    struct ComputationCompare;

    using SymbolList        = Gaston::SymbolList;
    using Symbol            = Gaston::Symbol;
    using VarType           = Gaston::VarType;
    using ValType           = Gaston::VarValue;

    using CacheData         = Term*;
    using SymbolKey         = std::tuple<Symbol*, VarType, ValType>;
    using SymbolHash        = boost::hash<SymbolKey>;
    using SymbolCompare     = std::equal_to<SymbolKey>;
    using BaseKey           = VATA::Util::OrdVector<size_t>;
    using BaseHash          = boost::hash<BaseKey>;
    using BaseCompare       = std::equal_to<BaseKey>;
    using ProductKey        = std::pair<Term*, Term*>;
    using ProductHash       = boost::hash<ProductKey>;
    using ProductCompare    = PairCompare<ProductKey>;
    using ListKey           = Term*;
    using ListHash          = boost::hash<ListKey>;
    using ListCompare       = std::equal_to<ListKey>;
    using FixpointKey       = std::pair<Term*, Symbol*>;
    using FixpointHash      = boost::hash<FixpointKey>;
    using FixpointCompare   = PairCompare<FixpointKey>;
    using Term_ptr          = Term*;
    using FixpointType      = std::list<std::pair<Term*,bool>>;
    using WorklistItemType  = std::pair<Term*, Symbol*>;
    using WorklistType      = std::list<WorklistItemType>;
    using ComputationKey    = TermFixpoint*;

    void dumpBaseKey(BaseKey const&);
    void dumpProductKey(ProductKey const&);
    void dumpListKey(ListKey const&);
    void dumpFixpointKey(FixpointKey const&);
    void dumpComputationKey(ComputationKey const&);
    void dumpCacheData(CacheData &);
    void dumpSymbolKey(SymbolKey const&);
    void dumpSymbolData(Symbol* &);

    using SymbolCache       = BinaryCache<SymbolKey, Symbol*, SymbolHash, SymbolCompare, dumpSymbolKey, dumpSymbolData>;
    using BaseCache         = BinaryCache<BaseKey, CacheData, BaseHash, BaseCompare, dumpBaseKey, dumpCacheData>;
    using ProductCache      = BinaryCache<ProductKey, CacheData, ProductHash, ProductCompare, dumpProductKey, dumpCacheData>;
    using ListCache         = BinaryCache<ListKey, CacheData, ListHash, ListCompare, dumpListKey, dumpCacheData>;
    using FixpointCache     = BinaryCache<FixpointKey, CacheData, FixpointHash, FixpointCompare, dumpFixpointKey, dumpCacheData>;
    using ComputationCache  = BinaryCache<ComputationKey, CacheData, ComputationHash, ComputationCompare, dumpComputationKey, dumpCacheData>;

    class TermWorkshop {
    private:
        // <<< PRIVATE MEMBERS >>>
        BaseCache* _bCache;
        ProductCache* _pCache;
        ListCache* _lCache;
        ListCache* _fpCache;
        FixpointCache* _fppCache;
        FixpointCache* _contCache;
        ComputationCache* _compCache;

        static TermEmpty *_empty;
        static TermEmpty *_emptyComplement;

        SymbolicAutomaton* _aut; // ProjectionAutomaton for Fixpoints
    private:
        // <<< PRIVATE FUNCTIONS >>>
        template<class A, class B, class C, class D, void (*E)(const A&), void (*F)(B&)>
        inline static BinaryCache<A, B, C, D, E, F>* _cleanCache(BinaryCache<A, B, C, D, E, F>*, bool noMemberDelete = false);

    public:
        // <<< CONSTRUCTORS >>>
        explicit TermWorkshop(SymbolicAutomaton*);
        ~TermWorkshop();
        void InitializeWorkshop();

        // <<< PUBLIC API >>>
        static TermEmpty* CreateEmpty();
        static TermEmpty* CreateComplementedEmpty();
        Term* CreateBaseSet(BaseKey &states, unsigned int offset, unsigned int stateno);
        TermProduct* CreateProduct(Term_ptr const&, Term_ptr const&, ProductType);
        TermFixpoint* CreateFixpoint(Term_ptr const&, Symbol*, bool, bool, WorklistSearchType search = WorklistSearchType::E_DFS);
        TermFixpoint* CreateFixpointPre(Term_ptr const&, Symbol*, bool);
        TermFixpoint* GetUniqueFixpoint(TermFixpoint*&);
        TermList* CreateList(Term_ptr const&, bool);
        Term* CreateContinuation(SymbolicAutomaton*, Term* const&, Symbol*, bool);

        void Dump();
    };

    class SymbolWorkshop {
    private:
        SymbolCache* _symbolCache = nullptr;
        SymbolCache* _trimmedSymbolCache = nullptr;
        std::vector<Symbol*> _trimmedSymbols;
        Symbol* _CreateProjectedSymbol(Symbol*, VarType, ValType);

    public:
        static Symbol* _zeroSymbol;

        // <<< CONSTRUCTORS >>>
        SymbolWorkshop();
        ~SymbolWorkshop();

        static Symbol* CreateZeroSymbol();
        Symbol* CreateSymbol(Symbol*, VarType, ValType);
        Symbol* CreateTrimmedSymbol(Symbol*, Gaston::VarList*);
        Symbol* CreateRemappedSymbol(Symbol*, std::map<unsigned int, unsigned int>*);

        void Dump();
    };
}

#endif //WSKS_WORKSHOPS_H
