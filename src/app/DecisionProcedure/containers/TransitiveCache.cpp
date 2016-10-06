//
// Created by Raph on 05/10/2016.
//

#include "TransitiveCache.h"
#include "Term.h"

Node::Node(Term* term, TransitiveCache* transitiveCache) : _term(term), _cache(transitiveCache) {
#   if (DEBUG_TRANSITIVE_CACHE == true)
    this->_bddStr = term->DumpToDot(_cache->dotOut);
#   endif
}

bool TransitiveCache::LookUp(RelationKey const& key, Gaston::SubsumptionResultPair& data) {
    if(this->_cache.retrieveFromCache(key, data)) {
        return true;
    } else {
        // Check the graph
        Node* from = this->_LookUpNode(key.second);
        Node* to = this->_LookUpNode(key.first);

        bool isIn;
        if(from->_succs.size() > to->_preds.size()) {
            isIn = std::find_if(from->_succs.begin(), from->_succs.end(), [&to](Node* i) { return to == i; }) != from->_succs.end();
        } else {
            isIn = std::find_if(to->_preds.begin(), to->_preds.end(), [&from](Node* i) { return from == i; }) != to->_preds.end();
        }

        if(isIn) {
            data.first = SubsumedType::YES;
            data.second = nullptr;
        }

        return isIn;
    }
}

void TransitiveCache::StoreIn(RelationKey const& key, Gaston::SubsumptionResultPair& data) {
    assert(data.first == SubsumedType::YES);
    // Assert that it is not in graph already
    this->_cache.StoreIn(key, data);
    this->_AddEdgeWithTransitiveClosure(key.second, key.first);
}

void TransitiveCache::_AddEdgeWithTransitiveClosure(Term* from, Term* to) {
    Node* from_node = this->_LookUpNode(from);
    Node* to_node = this->_LookUpNode(to);
    this->_AddEdgeWithTransitiveClosure(from_node, to_node);
}

void TransitiveCache::_AddEdge(Node* from, Node* to) {
    from->_succs.insert(to);
    to->_preds.insert(from);
}

void TransitiveCache::_AddEdgeWithTransitiveClosure(Node* from, Node* to) {
    // Create link between @p from and @p to
    this->_AddEdge(from, to);
#   if (DEBUG_TRANSITIVE_CACHE == true)
    this->dotOut << "\t" << from->_bddStr << " -> " << to->_bddStr << "\n";
#   endif

    // Create links to achieve transitivity
    Term_ptr ptr = nullptr;
    auto data = std::make_pair(SubsumedType::YES, ptr);
    for(Node* pred : from->_preds) {
        for(Node* succ : to->_succs) {
            this->_AddEdge(pred, succ);
            this->_cache.StoreIn(std::make_pair(pred->_term, succ->_term), data);
        }
    }
}

Node* TransitiveCache::_LookUpNode(Term* const& key) {
    auto it = this->_keyToNodeMap.find(key);
    Node* node;
    if(it != this->_keyToNodeMap.end()) {
        node = it->second;
    } else {
        node = TransitiveCache::_nodePool.construct(key, static_cast<TransitiveCache*>(this));
        this->_keyToNodeMap.emplace(key, node);
    }
    return node;
}

boost::object_pool<Node> TransitiveCache::_nodePool;
size_t TransitiveCache::cacheCount = 0;