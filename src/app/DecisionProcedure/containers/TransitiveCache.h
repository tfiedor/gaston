//
// Created by Raph on 05/10/2016.
//

#ifndef WSKS_TRANSITIVECACHE_H
#define WSKS_TRANSITIVECACHE_H

#include <boost/pool/object_pool.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_set>
#include "SymbolicCache.hh"
#include "../environment.hh"
#include <ostream>
#include <fstream>
#include <sstream>

#define DEBUG_TRANSITIVE_CACHE false

class Term;
class TransitiveCache;

using RelationKey = std::pair<Term*, Term*>;

struct Node {
    Term* _term;
    std::unordered_set<struct Node*> _preds;
    std::unordered_set<struct Node*> _succs;
    size_t* _region = nullptr;
#   if (DEBUG_TRANSITIVE_CACHE == true)
    TransitiveCache* _cache;
    std::string _bddStr;
#   endif
    static boost::object_pool<size_t> regionPool;
    static size_t regionNumber;

#   if (DEBUG_TRANSITIVE_CACHE == true)
    Node(Term* term, TransitiveCache* transitiveCache);
#   else
    Node(Term* term);
#   endif
    static void ClassifyRegions(Node*, Node*);
};


struct RelationKeyCompare : public std::binary_function<RelationKey, RelationKey, bool> {
    bool operator()(RelationKey const& lhs, RelationKey const& rhs) const {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};

struct RelationKeyHash {
    size_t operator()(RelationKey const& key) const {
        size_t seed = boost::hash_value(key.first);
        boost::hash_combine(seed, boost::hash_value(key.second));
        return seed;
    }
};

class TransitiveCache {
    friend struct Node;
public:
    TransitiveCache() {
        ++TransitiveCache::cacheCount;
#       if (DEBUG_TRANSITIVE_CACHE == true)
        std::string name("transitiveCache");
        name += std::to_string(TransitiveCache::cacheCount);
        name += ".dot";
        dotOut.open(name);
        dotOut << "strict digraph aut {\n";
#       endif
    }
    ~TransitiveCache() {
#       if (DEBUG_TRANSITIVE_CACHE == true)
        dotOut << "}\n";
#       endif
    }

    std::ofstream dotOut;
    static size_t cacheCount;
private:
    BinaryCache<RelationKey, Gaston::SubsumptionResultPair, RelationKeyHash, RelationKeyCompare, Gaston::dumpSubsumptionKey, Gaston::dumpSubsumptionPairData> _cache;
    using Term_ptr = Term*;
    using TermHash = boost::hash<Term_ptr>;
    using TermCompare = std::equal_to<Term_ptr>;
    static boost::object_pool<Node> _nodePool;

    Node* _LookUpNode(Term* const&);
    void _AddEdgeWithTransitiveClosure(Node*, Node*);
    void _AddEdge(Node*, Node*);
    void _AddEdgeWithTransitiveClosure(Term*, Term*);
public:
    bool LookUp(RelationKey const&, Gaston::SubsumptionResultPair&);
    void StoreIn(RelationKey const&, Gaston::SubsumptionResultPair&);
};


#endif //WSKS_TRANSITIVECACHE_H
