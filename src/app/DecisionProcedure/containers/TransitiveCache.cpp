//
// Created by Raph on 05/10/2016.
//

#include "TransitiveCache.h"
#include "Term.h"

#   if (DEBUG_TRANSITIVE_CACHE == true)
Node::Node(Term* term, TransitiveCache* transitiveCache) : _term(term), _cache(transitiveCache) {
    if(!term->IsIntermediate()) {
        this->_bddStr = term->DumpToDot(_cache->dotOut);
        if (term->link != nullptr && !term->IsIntermediate()) {
            Term_ptr iter = term->link->succ;
            while (iter != nullptr && iter->link->succ != nullptr && iter->IsIntermediate()) {
                iter = iter->link->succ;
            }

            if (iter != nullptr) {
                Node *pre = _cache->_LookUpNode(iter);
                _cache->dotOut << pre->_bddStr << " -> " << this->_bddStr << " [style=\"dashed\"]\n";
            }
        }
    }
}
#   else
Node::Node(Term *term) : _term(term) {

}
#   endif

void Node::ClassifyRegions(Node* from, Node* to) {
    assert(from != nullptr);
    assert(to != nullptr);

    bool from_is_null = from->_region == nullptr;
    bool to_is_null = to->_region == nullptr;
    if(from_is_null && to_is_null) {
        from->_region = Node::regionPool.construct();
        *from->_region = Node::regionNumber++;
        to->_region = from->_region;
    } else if(from_is_null) {
        // Align by to->region
        from->_region = to->_region;
    } else {
        // Align by from->region
        to->_region = from->_region;
    }

}

bool TransitiveCache::LookUp(RelationKey const& key, Gaston::SubsumptionResultPair& data) {
    if(key.first->node == nullptr || key.second->node == nullptr) {
        return false;
    } else if(key.first->node->_region != key.second->node->_region) {
        return false;
    } else if(key.first != key.second) {
        return this->_cache.retrieveFromCache(key, data);
    } else {
        data.first = SubsumedType::YES;
        data.second = nullptr;
        return true;
    }
}

void TransitiveCache::StoreIn(RelationKey const& key, Gaston::SubsumptionResultPair& data) {
    assert(data.first != SubsumedType::NOT);
    if(key.first != key.second) {
        // Assert that it is not in graph already
        this->_cache.StoreIn(key, data);
        if(data.first == SubsumedType::YES) {
            this->_AddEdgeWithTransitiveClosure(key.second, key.first);
        }
    }
}

void TransitiveCache::_AddEdgeWithTransitiveClosure(Term* from, Term* to) {
    assert(from != to);
    Node* from_node = this->_LookUpNode(from);
    Node* to_node = this->_LookUpNode(to);
    Node::ClassifyRegions(from_node, to_node);
    this->_AddEdgeWithTransitiveClosure(from_node, to_node);
}

void TransitiveCache::_AddEdge(Node* from, Node* to) {
    assert(from != to);
    from->_succs.insert(to);
    to->_preds.insert(from);
}

void TransitiveCache::_AddEdgeWithTransitiveClosure(Node* from, Node* to) {
    // Create link between @p from and @p to
    this->_AddEdge(from, to);
#   if (DEBUG_TRANSITIVE_CACHE == true)
    if(!from->_term->IsIntermediate()) {
        this->dotOut << "\t" << from->_bddStr << " -> " << to->_bddStr << "\n";
    }
#   endif

    // Create links to achieve transitivity
    Term_ptr ptr = nullptr;
    auto data = std::make_pair(SubsumedType::YES, ptr);
    for(Node* pred : from->_preds) {
        for(Node* succ : to->_succs) {
            if(pred == succ)
                continue;
            this->_AddEdge(pred, succ);
            this->_cache.StoreIn(std::make_pair(pred->_term, succ->_term), data);
        }
    }
}

Node* TransitiveCache::_LookUpNode(Term* const& key) {
    if(key->node == nullptr) {
#       if (DEBUG_TRANSITIVE_CACHE == true)
        key->node = TransitiveCache::_nodePool.construct(key, static_cast<TransitiveCache*>(this));
#       else
        key->node = TransitiveCache::_nodePool.construct(key);
#       endif
    }

    return key->node;
}

boost::object_pool<Node> TransitiveCache::_nodePool;
boost::object_pool<size_t> Node::regionPool;
size_t TransitiveCache::cacheCount = 0;
size_t Node::regionNumber = 0;