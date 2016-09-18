#ifndef MONAWRAPPER_H
#define MONAWRAPPER_H

#include "ondriks_mtbdd.hh"
#include "../../Frontend/ast.h"
#include "../containers/SymbolicCache.hh"
#include "../containers/VarToTrackMap.hh"
#include "../environment.hh"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <assert.h>
#include <string>
#include <fstream>
#include <boost/dynamic_bitset.hpp>
#include <boost/pool/object_pool.hpp>
#include <vata/util/ord_vector.hh>

extern VarToTrackMap varMap;

struct WrappedNode
{
    std::unordered_set<struct WrappedNode *> pred_[2];
    unsigned node_;
    int var_;

    WrappedNode(unsigned addr, int var = 0): node_(addr), var_(var)
    {
        ;
    }

    friend std::ostream &operator<<(std::ostream &out, const WrappedNode &rhs) {
        out << rhs.node_ << "(" << (rhs.var_ & 0x1ffff) << ")";
        return out;
    }
};

template<class Data>
class MonaWrapper
{
private:
    struct HashType
    {
        size_t operator()(std::pair<struct WrappedNode *, Gaston::BitMask> const& set) const
        {
    #		if (OPT_SHUFFLE_HASHES == true)
            size_t seedLeft = hash64shift(boost::hash_value(set.first));
            size_t seedRight = hash64shift(boost::hash_value(set.second.count()));
            boost::hash_combine(seedRight, hash64shift(boost::hash_value(set.second.find_first())));
    #		else
            size_t seedLeft = boost::hash_value(set.first);
            size_t seedRight = boost::hash_value(set.second.count());
            boost::hash_combine(seedRight, boost::hash_value(set.second.find_first()));
    #		endif

            boost::hash_combine(seedLeft, seedRight);
    #		if (OPT_SHUFFLE_HASHES == true)
            return hash64shift(seedLeft);
    #		else
            return seedLeft;
    #		endif
        }
    };

    template<class Key>
    struct PairCompare : public std::binary_function<Key, Key, bool>
    {
        bool operator()(Key const& lhs, Key const& rhs) const {
            return (lhs.first == rhs.first) && (lhs.second == rhs.second);
        }
    };

    using DataType          = Data;
    using VectorType        = VATA::Util::OrdVector<Data>;
    using WrappedNode       = struct WrappedNode;
    using InternalNodesType = std::unordered_map<unsigned, WrappedNode *>;
    using SetType           = std::unordered_set<WrappedNode *>;

    using KeyType			= std::pair<WrappedNode *, Gaston::BitMask>;
    using CacheType			= BinaryCache<KeyType, VectorType, HashType, PairCompare<KeyType>, Gaston::dumpKey, Gaston::dumpData>;

protected:
    std::vector<WrappedNode *> roots_;
    std::vector<WrappedNode *> leafNodes_;
    InternalNodesType internalNodes_;
    DFA *dfa_;
    unsigned numVars_;
    size_t initialState_;
    Gaston::BitMask symbol_;
    Gaston::BitMask mask_;
    Gaston::BitMask* masks_;
    Gaston::BitMask* symbolMasks_;
    unsigned exploredState_;
    bool isRestriction_ = false;
    CacheType cache_;
    static boost::object_pool<WrappedNode> nodePool_;

public:
    const int LEAF_VAR_VALUE = 0xfffffffe;
    static int _wrapperCount;
#   if (MEASURE_AUTOMATA_CYCLES == true)
    static size_t noAllFinalStatesHasCycles;    // < Number of wrapped automata, that satisfy the condition
    static size_t noSomeFinalStatesHasCycles;   // < Number of wrapped automata that satisfy this condition
    static size_t noAllStatesHasSomeCycles;     // < Number of wrapped automata that satisfy this condition
    static size_t someStatesHasSomeCycles;      // < Cumulative awerage that satisfy this condition
#   endif

private:
    /**
     * @brief Creates a wrapper for the node corresponding to the @p addr
     *
     * Looks up the node corresponding to the @p addr. If no such node exists,
     *   new one is spawned. Further an edge is created leading to the @p pred
     *   of the node.
     *
     * @param[in]  addr  address of the node we are spawning
     * @param[out]   pred  successor of the node, that we are spawning
     * @param[in]  edge  either 0 or 1 edge in BDD
     * @return  wrapped DFA node
     */
    WrappedNode *spawnNode(unsigned addr, WrappedNode &pred, bool edge)
    {
        WrappedNode *node;
        typename InternalNodesType::iterator itN;

        if((itN = internalNodes_.find(addr)) != internalNodes_.end())
        {
            // Node exists
            node = itN->second;
        }
        else
        {
            node = MonaWrapper<Data>::nodePool_.construct(addr);
            internalNodes_.insert(std::make_pair(addr, node));
        }

        // Add new predecessor of the node
        node->pred_[edge].insert(&pred);
        return node;
    }

    /**
     * @brief Spawns node for the @p state
     *
     * @param[in]  bddm  BDD manager of the DFA representation
     * @param[in]  addr  address of the node we are spawning
     * @param[in]  state  state for which we are spawning the wrapped node
     * @return  wrapped DFA node
     */
    WrappedNode *spawnNode(const bdd_manager *bddm, unsigned addr, size_t state)
    {
        unsigned index;

        leafNodes_[state] = MonaWrapper<Data>::nodePool_.construct(state, this->LEAF_VAR_VALUE);
        LOAD_index(&bddm->node_table[addr], index);

        // Fixme: The fuck is this cock magic?
        if(index != BDD_LEAF_INDEX && varMap[index] == 0)
            return leafNodes_[state];

        // ? If we have self loop on the state, this handles the case I guess?
        WrappedNode *result = spawnNode(addr, *leafNodes_[state], 1);
        result->pred_[0].insert(leafNodes_[state]);
        return result;
    }

    /**
     * @brief Recursive construction of wrapping over the MONA DFA
     *
     * @param[in]  bddm  BDD manager of MONA // Fixme: Isn't this unique through the computation? i.e. in this->dfa_??
     * @param[in]  p  address of the BDD node in MONA
     * @param[out]  previously wrapped node
     */
    void RecSetPointer(const bdd_manager *bddm, unsigned p, WrappedNode &node)
    {
        unsigned l, r, index;

        LOAD_lri(&bddm->node_table[p], l, r, index);
        if (index == BDD_LEAF_INDEX)
        {
            if(roots_[l] == nullptr)
            {
                node.var_ = SetVar(numVars_);
                roots_[l] = &node;
            }
        }
        else
        {
            node.var_ = SetVar(varMap[index]);
            RecSetPointer(bddm, l, *spawnNode(l, node, 0));
            RecSetPointer(bddm, r, *spawnNode(r, node, 1));
        }
    }

    /**
     * @brief Returns true if the @p var corresponds to LEAF value
     *
     * Note: Yeah i know this is stupid cock magic, but whatever
     * Fixme: This is not exactly true, there
     *
     * @param[in]  var  variable we are checking for leaves
     * @return  true if the @p var is leaf
     */
    inline bool IsLeaf(int var) {
        return GetVar(var) == GetVar(this->LEAF_VAR_VALUE);
    }

    /**
     * @brief Retrives from @p var the part that corresonds the variables
     *
     * @param[in]  var  stored variable and flags
     * @return  part of the variable that corresponds to variable
     */
    inline int GetVar(int var)
    {
        return var & 0x1ffff;
    }

    /**
     * @brief Checks if var1 equals var2
     *
     * @param[in]  var1  lhs of the equality testing
     * @param[in]  var2  rhs of the equality testing
     * @return  true if @p var1 equals @p var2
     */
    inline bool UnequalVars(int var1, int var2)
    {
        return GetVar(var1) ^ GetVar(var2);
    }

    /**
     * Fixme: Ok seriously what? This sets maybe yo mama!
     */
    inline int SetVar(unsigned index)
    {
        return (index - 1) | 0x00060000;
    }

    /**
     * @brief Sets in @p var that previous predecessor was true or false
     *
     * @param[out]  var  storage for flags
     * @param[in]  edge  value of the edge (0 or 1)
     */
    inline void SetFlag(int &var, bool edge)
    {
        var = var & (0xfffbffff >> edge);
    }

    /**
     * @brief Returns true if the predecessor of previous computation was high
     *
     * During the traversal we skip some nodes by 'X' values. We need to know,
     *   which edge was taken on higher level though. This does it.
     *
     * @param[in]  var  stored flags
     * @return  true if the predecessor of previous computation was 1
     */
    inline bool IsPredecessorHigh(int var)
    {
        return (var & 0x00020000) ^ 0x00020000;
    }

    /**
     * @brief Returns true if the predecessor of previous computation was low
     *
     * During the traversal we skip some nodes by 'X' values. We need to know,
     *   which edge was taken on higher level though. This does it.
     *
     * @param[in]  var  stored flags
     * @return  true if the predecessor of previous computation was 0
     */
    inline bool IsPredecessorLow(int var)
    {
        return (var & 0x00040000) ^ 0x00040000;
    }

    /**
     * @brief Resets the flags in @p var.
     *
     * Flags on node are stored in 2-bits somewhere in the middle. The concrete
     *   position was obtained by sacrificing twelve goats during the fool moon
     *   while chanting the lyrics of Shake It Off by Taylor Swift in Portuguese.
     *   The minute we went crazy gave us the position.
     * Note: The Flags are reseted as 11 (not 00, like usual)
     *
     * @param[out]  integer that stores the flags
     */
    inline void ResetFlags(int &var)
    {
        var = var | 0x00060000;
    }

    /**
     * @brief Converses and optionaly serializes @p nodes to vector type
     *
     * Takes the nodes from @p nodes, optionally serializes them and adds them to
     *   the vector.
     *
     * @param[in]  nodes  nodes we are flattening to vector
     * @param[in]  serialize_value  true if the values should be serialized
     * @return  nodes in vector
     */
    inline VectorType CreateResultSet(const SetType &nodes, bool serialize_value = false)
    {
        VectorType vec;

        for(auto node: nodes) {
            if(serialize_value) {
                vec.insert(this->_SerializeNode(node->node_, node->var_));
            } else {
                vec.insert(node->node_);
            }
            ResetFlags(node->var_);
        }

        return vec;
    }

    /**
     * @brief Get all of the Don't Care successors into @p succs
     * Fixme: What is the difference between this and GetSuccessors? Like really.
     *
     * Retrieves all nodes from @p nodes into @p succs, after taking the 'X' edge
     *   on the @p var level. No idea why the fuck is there @p edge, but i guess
     *   it is because we processed different level on top of the don't care.
     *
     * @param[in]  nodes  nodes we are adding to @p succs
     * @param[out]  succs  collection of nodes
     * @param[in]  var  currently processed variable level
     * @param[in]  edge  edge we took on top
     */
    inline void GetDontCareSucc(const SetType &nodes, SetType &succs, int var, bool edge)
    {
        for(auto node: nodes)
        {
            if(UnequalVars(node->var_, var))
            {
                succs.insert(node);
                SetFlag(node->var_, edge);
            }
        }
    }

    /**
     * @brief Collect all nodes from @p nodes into @p succs, and sets the flags if we are on different level
     *
     * Collects all of the nodes from @p nodes (either pred[0] or pred[1]),
     *   into the @p succs. If the collected node is on different level, than
     *   we are currently processing (i.e. @p var), we flag the node which edge
     *   we took. (This is hackish optimization/solution)
     *
     * @param[in]  nodes  nodes we are collecting successors from
     * @param[out]  succs  container where we collect states
     * @param[in]  var  variable level we are currently processing
     * @param[in  edge  edge we took (either 0 = false, or 1 = true)
     */
    inline void GetSuccessors(const SetType &nodes, SetType &succs, int var, bool edge)
    {
        for(auto node: nodes)
        {
            assert(node != nullptr);
            succs.insert(node);
            if(UnequalVars(node->var_, var))
                SetFlag(node->var_, edge);
        }
    }

    /**
     * @brief transforms the @p symbol into sub path starting from @p var level
     *
     * Takes the symbol @p symbol and according to the current level @p var,
     *   transforms the symbol into subpath, that is used in cache.
     *
     * @param[in]  symbol  symbol we are transforming into subpath
     * @param[in]  var  variable level, where the subpath starts
     * @return  transformed symbol corresponding to subpath
     */
    inline Gaston::BitMask transformSymbol(const Gaston::BitMask &symbol, int var)
    {
        assert(var < this->numVars_);
        Gaston::BitMask transformed = symbol;
        transformed &= symbolMasks_[var];
        transformed |= masks_[var];
        return transformed;
    }

    /**
     * @brief Recursively traverse the BDD, computing the pre of states in DFS manner
     *
     * Traverses the BDD of MONA, that is wrapped by DJPJ's mighty Wrapper.
     * Works in Depth First Search manner, i.e. it computes the Pre by traversing
     *   each path in the BDD, exploiting the cache for already traversed subpaths.
     *
     * @param[in]  node  starting node of the search
     * @param[out]  res  accumulated results
     */
    void RecPre(WrappedNode *node, VectorType &res)
    {
        // Checks if we are in the leaf level. If yes, add the value of the leaf
        if(this->IsLeaf(node->var_) || this->IsLeaf(node->var_-1) || node->var_ < 0) {
        // Previously: if(node->var_ < 0) {
            res.insert(node->node_);
            return;
        }

        int var = GetVar(node->var_);

#       if (OPT_CACHE_SUBPATHS_IN_WRAPPER == true)
        // Checks if we already explored (node + subpath) in cache
        VectorType tmp;
        auto key = std::make_pair(node, transformSymbol(symbol_, var));

        if(cache_.retrieveFromCache(key, tmp))
        {
            res.insert(tmp);
            return;
        }
#       endif

        VectorType innerRes;
        if(/*symbol_[var << 1] &&*/ symbol_[(var << 1) + 1])
        {
            // If the value of the symbol on [var] level is 'X', i.e. don't care
            for(auto nnode: node->pred_[~symbol_[(var << 1)]])
            // Fixme:                   ^--- why the fuck do we complement the whole symbol, to get the inverse shit
            // Fixme: (-.-)'
            {
                if(UnequalVars(nnode->var_, var - 1) && !this->IsLeaf(nnode->var_))
                {
                    if(~symbol_[(var << 1)] == symbol_[(GetVar(nnode->var_ + 1) << 1)] ||
                        symbol_[(GetVar(nnode->var_ + 1) << 1) + 1])
                        RecPre(nnode, innerRes);
                }
                else
                {
                    RecPre(nnode, innerRes);
                }
            }
        }
        else
        {
            for(auto nnode: node->pred_[~symbol_[(var << 1)]])
                if(UnequalVars(nnode->var_, var - 1) && !this->IsLeaf(nnode->var_))
                    if((~symbol_[(var << 1)] == symbol_[(GetVar(nnode->var_ + 1) << 1)]) ||
                       symbol_[(GetVar(nnode->var_ + 1) << 1) + 1])
                        RecPre(nnode, innerRes);
        }

        for(auto nnode: node->pred_[symbol_[(var << 1)]])
        {
            if(UnequalVars(nnode->var_, var - 1) && !this->IsLeaf(nnode->var_))
            {
                if(symbol_[(var << 1)] == symbol_[(GetVar(nnode->var_ + 1) << 1)] ||
                   symbol_[(GetVar(nnode->var_ + 1) << 1) + 1])
                    RecPre(nnode, innerRes);
            }
            else
            {
                RecPre(nnode, innerRes);
            }
        }

        if(innerRes.size() != 0) {
            res.insert(innerRes);
        }
#       if (OPT_CACHE_SUBPATHS_IN_WRAPPER == true)
        cache_.StoreIn(key, innerRes);
#       endif
    }

    /**
     * @brief Ascends by one @p var level simulating the 'X' value
     *
     * Ascends by one @p var level through the pre starting from @p source nodes,
     *   accumulating results to @p dest nodes. This accumulates both the 0 and 1
     *   predecessors and simulates the 'X' behaviour in symbol
     *
     * @param[in]  var  level in the bdd we are ascending by
     * @param[in]  source  source set of bdd nodes
     * @param[out]  dest  destination set of bdd nodes
     */
    void AscendByLevel(size_t var, SetType& source, SetType& dest) {
        for(auto node: source) {
            assert(node != nullptr);
            if (UnequalVars(node->var_, var)) {
                // Variables are unequal, we wait to synchronize with the rest
                assert(node->var_ > var);
                dest.insert(node);
            } else {
                // Ascend by both 0 and 1
                GetSuccessors(node->pred_[0], dest, var - 1, 0);
                GetSuccessors(node->pred_[1], dest, var - 1, 1);
                ResetFlags(node->var_);
            }
        }
    }

    /**
     * @brief Ascends by one @p var level simulating the '0' or '1' value
     *
     * Ascends by one @p var level through the pre starting from @p source nodes,
     *   accumulating results to @p dest nodes. This ascension is limited to the
     *   @p val.
     *
     * @param[in]  var  level in the bdd we are ascending by
     * @param[in]  val  value which limits the ascension
     * @param[in]  source  source set of bdd nodes
     * @param[out]  dest  destination set of bdd nodes
     */
    void AscendByLevelByValue(size_t var, bool val, SetType& source, SetType& dest) {
        // not "don't care".
        for(auto node: source) {
            std::cout << "Ascending from: " << (*node) << "\n";
            assert(node != nullptr);
            if(GetVar(node->var_) == var) {
                // There could be don't care on the lower level so we include those stuff
                GetDontCareSucc(node->pred_[!val], dest, var - 1, !val);
                // Get the usual symbol stuff
                GetSuccessors(node->pred_[val], dest, var - 1, val);
                ResetFlags(node->var_); // Fixme: <--- This could pose problems i think
            } else {  // don't care on(at?) created node.
                if(UnequalVars(node->var_, var - 1)) {
                    // We wait to synchronize
                    dest.insert(node);
                } else {
                    // We got here from some higher don't care
                    if((val && IsPredecessorHigh(node->var_)) ||
                       (!val && IsPredecessorLow(node->var_)))
                        dest.insert(node);

                    ResetFlags(node->var_);
                }
            }
            ResetFlags(node->var_);
        }
    }

    /**
     * @brief Recursively traverse the BDD, computing the pre of states BFS manner, by variable levels
     *
     * Traverses the BDD of MONA, that is wrapped by DJPJ's mighty Wrapper.
     * Works in Breadth First Search manner, i.e. it computes the Pre by
     *   traversing the levels corresponding to variables.
     *
     * @param[in]  nodes  starting nodes of the recursive pre
     * @return  reached nodes through the symbol
     */
    VectorType RecPre(SetType& nodes)
    {
        //SetType nodes({root});
        SetType res;

        int var = numVars_ - 1;
        for(; -1 < var; --var)
        {
            if(/*symbol_[var << 1] &*/ symbol_[(var << 1) + 1]) {
                AscendByLevel(var, nodes, res);
            } else {
                AscendByLevelByValue(var, symbol_[(var << 1)], nodes, res);
            }
            nodes = std::move(res);
        }

        return CreateResultSet(nodes);
    }

    /**
     * @brief Serializes the node into single size_t value
     *
     * Takes the internals of the BDD node and serializes in into one size_t value. Serialized value
     *   consists of node address and 2-bits of flags corresponding to the traversals of the bdd.
     *
     * @param[in]  addr  address of serialized node in BDD graph
     * @param[in]  flags  value containing the flags for the node
     * @return  serialized node
     */
    size_t _SerializeNode(int addr, int flags) {
        size_t serialized = (addr << 2);
        if(this->IsPredecessorLow(flags)) {
            serialized += 1;
        } else if(this->IsPredecessorHigh(flags)) {
            serialized += 2;
        }
        return serialized;
    }

    /**
     * @brief Deserialize the value into WrappedNode object
     *
     * Takes the serialized integer value, that was outputed to the TermBaseSet,
     *   and deserializes it into address and flags.
     *
     * @param[in]  serialized  serialized value
     * @return  WrappedNode corresponding to serialized value with set flags
     */
    WrappedNode* _DeserializeValue(size_t serialized) {
        WrappedNode* node = nullptr;

        // Retrieve flags and address
        bool is_low = (serialized & 1);
        bool is_high = (serialized & 2);
        serialized >>= 2;

        if(serialized > this->dfa_->ns) {
            node = this->internalNodes_[serialized];
        } else {
            node = this->leafNodes_[serialized];
        }
        assert(node != nullptr);

        // Restore flags
        if(is_low) {
            this->SetFlag(node->var_, false);
        } else if(is_high) {
            this->SetFlag(node->var_, true);
        }

        return node;
    }

    /**
     * @brief Looks up the node corresponding to @p state value
     *
     * Looks up node for the value of @p state. Either a root node, that is stored
     *   in roots_, or internal node from internalNodes_. Internal nodes have addresses
     *   greater than numvar.
     * Fixme: Could there be a internal node that does fails this assumption?
     *
     * @param[in]  state  value for which we are looking the node up
     * @return  internal or root node corresponding to @p state
     */
    WrappedNode* _LookupNode(size_t state) {
        if(state > this->dfa_->ns) {
            // Add internal node
            return this->internalNodes_[state];
        } else {
            // Skip the nullptr (unused) states
            if(roots_[state] != nullptr) {
                return roots_[state];
            }
        }

        return nullptr;
    }

public:
    /**
     * Constructs the MonaWrapper that serves as intermediate layer between
     *   the logic of Gaston and the internal representation of MONA. Since
     *   MONA works in post manner, this wrapper transforms the Post on transitions
     *   to corresponding Pres on transition.
     *
     * @param[in]  dfa  deterministic finite automaton, in MONA format
     * @param[in]  emptyTracks  whether the tracks are fully projected (corresponds to true/false formulae)
     * @param[in]  is_restriction  true if the DFA is restriction (see restriction semantics)
     * @param[in]  numVars  number of variables in automaton (corresponds to bdd levels)
     */
    MonaWrapper(DFA *dfa, bool emptyTracks, bool is_restriction, unsigned numVars = 0)
            : dfa_(dfa), numVars_(numVars), initialState_(emptyTracks ? 0 : 1), isRestriction_(is_restriction)
    {
        // Resize the roots and leaves
        roots_.resize(dfa->ns, nullptr);
        leafNodes_.resize(dfa->ns, nullptr);

        // Create a mask for the optimized pre
        mask_.resize(numVars_ << 1);
        for(unsigned i = 0; i < numVars_; ++i)
            mask_[(i << 1) + 1] = true;

        Gaston::BitMask symbolMask;
        symbolMask.resize(numVars_ << 1);
        symbolMask.set();

        // Additional masks used in TransformSymbol
        this->masks_ = new Gaston::BitMask[numVars];
        this->symbolMasks_ = new Gaston::BitMask[numVars];
        for(unsigned i = 0; i < numVars_; ++i) {
            unsigned int diff = ((numVars_ - 1) - i) << 1;

            this->symbolMasks_[i] = symbolMask;
            this->symbolMasks_[i] >>= diff;

            this->masks_[i] = mask_;
            this->masks_[i] <<= ((i+1) << 1);
        }


        for(size_t i = this->initialState_; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));

        ++MonaWrapper<Data>::_wrapperCount;
#       if (DEBUG_MONA_BDD == true)
        this->DumpToDot(std::string("bdd") + std::to_string(MonaWrapper<Data>::_wrapperCount) + std::string(".dot"));
#       endif
    }

    ~MonaWrapper()
    {
        delete[] this->masks_;
        delete[] this->symbolMasks_;
        dfaFree(this->dfa_);
    }

    /**
     * @brief Serializes the BDD into .dot file
     *
     * @param[in]  outfile  output file for the bdd
     */
    void DumpToDot(std::string outfile)
    {
        std::ofstream ofs (outfile, std::ofstream::out);

        ofs << "digraph M {" << std::endl;

        ofs << "subgraph cluster0 {" << std::endl;
        ofs << "color=\"#008000\";" << std::endl;
        for(size_t i = 1; i < roots_.size(); i++)
            ofs << "s" << i << ";" << std::endl;
        ofs << "}" << std::endl;

        ofs << "subgraph cluster1 {" << std::endl;
        ofs << "color=\"#800000\";" << std::endl;
        for(auto leaf: leafNodes_) {
            if(leaf == nullptr) {
                std::cerr << "[!] Warning: Empty leaf\n";
                continue;
            }
            ofs << "t" << leaf->node_ << "[label=\"" << leaf->node_ << " (" << GetVar(leaf->var_) << ")\"];" <<
            std::endl;
        }
        ofs << "}" << std::endl;

        for(auto node: internalNodes_)
            ofs << node.first << "[label=\"" << node.first << " (" << GetVar(node.second->var_) << ")\"];" << std::endl;

        for(size_t i = this->initialState_; i < roots_.size(); i++) {
            if(roots_[i] == nullptr)
                continue;
            ofs << "s" << i << " -> " << roots_[i]->node_ << ";" << std::endl;
        }

        for(auto node: internalNodes_)
        {
            for(auto low: node.second->pred_[0])
            {
                if(low->pred_[0].size() || low->pred_[1].size())
                    ofs << node.first << " -> " << low->node_ << " [style=dotted];" << std::endl;
                else
                    ofs << node.first << " -> t" << low->node_ << " [style=dotted];" << std::endl;
            }

            for(auto high: node.second->pred_[1])
            {
                if(high->pred_[0].size() || high->pred_[1].size())
                    ofs << node.first << " -> " << high->node_ << std::endl;
                else
                    ofs << node.first << " -> t" << high->node_ << std::endl;
            }
        }

        ofs << "}" << std::endl;
    }

    /**
     * @brief Recursively constructs the transitions for the MONA DFA representation
     * // Fixme: This name is stupid
     *
     * Recursively constructs all the paths leading from @p root to the leaf nodes.
     *   Transitions are stored in @p transition, which is slowly constructed
     *   according to the informations from the @p bddm manager.
     *
     * @param[out]  os  output stream
     * @param[in]  bddm  bdd manager of the MONA automaton
     * @param[in]  p  adddress of the bdd node
     * @param[out]  transition  output for the transition, that is recursively created
     * @param[in]  root  root of the transition
     * @praram[in]  varNum  number of variables in bdd // Fixme: This is redundant, right?
     */
    void GetAllPathFromMona(std::ostream &os,
                            const bdd_manager *bddm,
                            unsigned p,
                            std::string transition,
                            size_t root,
                            size_t varNum)
    {
        unsigned l, r, index;

        LOAD_lri(&bddm->node_table[p], l, r, index);

        if (index == BDD_LEAF_INDEX)
        {
            // We encountered leaf node -> output the transition
#           if (PRINT_IN_TIMBUK == true)
            os << transition << "(" << root << ") -> " << l << "\n";
#           else
            os << root << " -(" << transition << ")-> " << l << "\n";
#           endif
        }
        else
        {
            // We recursively construct the paths for 0 and 1 symbol values.
            transition[varMap[index]] = '0';
            GetAllPathFromMona(os, bddm, l, transition, root, varNum);

            transition[varMap[index]] = '1';
            GetAllPathFromMona(os, bddm, r, transition, root, varNum);
        }
    }

    /**
     * @brief Outputs all transitions to @p os
     *
     * For each state of the DFA constructs all outgoing paths and outputs them to @p os.
     *
     * @param[out]  os  output
     */
    void DumpDFA(std::ostream &os) {
        std::string str(this->numVars_, 'X');
        for(size_t i = this->initialState_; i < this->dfa_->ns; ++i) {
            GetAllPathFromMona(os, this->dfa_->bddm, this->dfa_->q[i], str, i, this->numVars_);
        }
    }

    /**
     * @brief Outputs the automaton in timbuk format.
     *
     * @param[out]  os  output stream for the dfa automaton
     */
    void DumpTo(std::ostream &os) {
        os << "Ops\n";
        os << "Automaton anonymous\n";
        os << "States";
        for(size_t i = this->initialState_; i < this->dfa_->ns; ++i) {
            os << " " << i;
        }
        os << "\n";
        os << "Final States";
        for(size_t i = this->initialState_; i < this->dfa_->ns; ++i) {
            if(this->dfa_->f[i] == 1) {
                os << " " << i;
            }
        }
        os << "\n";
        os << "Transitions\n";
        DumpDFA(os);
    }

    /**
     * @brief Public API for computing pre of the @p state through @p symbol in DFS manner
     *
     * Constructs the pre of the @p state through the @p symbol, by iterating the path
     *   by Depth First Search manner.
     *
     * @param[in]  state  state we are traversing from
     * @param[in]  symbol  symbol through which we are iterating
     * @return  vector of base states reachable from @p state through @p symbol
     */
    VectorType Pre(size_t state, const boost::dynamic_bitset<> &symbol)
    {
        assert(dfa_ != nullptr);
        assert(roots_.size() > state);

        // Null roots reach nothing, John Snow
        if(roots_[state] == nullptr)
            return VectorType();

        symbol_ = symbol;

        // Call internal Recursive Pre computation
        VectorType res;
        RecPre(roots_[state], res);

        return res;
    }

    /**
     * @brief Public API for computing pre of the @p states through @p symbol in BFS manner
     *
     * Enqueues all the states from @p states, and by each variable level,
     *   computes the pre through the @p symbol.
     *
     * @param[in]  states  starting set of states
     * @param[in]  symbol  symbol through which we are iterating
     * @return  vector of base states reachable from @p states through @p symbol
     */
    VectorType Pre(VATA::Util::OrdVector<size_t> &states, const boost::dynamic_bitset<> &symbol)
    {
        assert(dfa_ != nullptr);
        SetType nodes;

        // Translate base states to internal nodes
        for(auto state: states)
        {
            assert(roots_.size() > state);
            if(roots_[state] != nullptr)
                nodes.insert(roots_[state]);
        }

        if(nodes.empty())
            return VectorType();

        // Call core function
        symbol_ = symbol;
        return RecPre(nodes);
    }

    /**
     * @brief Public API for computing Pre from @p states through symbol by ascending by @p val on variable level @p var
     *
     * Novel super Pre, that only ascends by one @p var level, that can abe limited to @p val,
     *   which is either 1 or 0.
     *
     * @param[in]  states  starting point of the pre
     * @param[out]  symbol  output path (reconstructed symbol)
     * @param[in]  var  variable level we are ascending from
     * @param[in]  val  limitation of the ascension (either 1 or 0)
     * @return  vector of states reachable from @p states by ascending from @p var level
     */
    VectorType Pre(VATA::Util::OrdVector<size_t> &states, const boost::dynamic_bitset<> &symbol, size_t var, char val) {
        assert(var < this->numVars_ || this->numVars_ == 0);
        assert(dfa_ != nullptr);

        SetType sources, dest;
        WrappedNode* node;
        for(auto state : states) {
            // Root level variables are not serialized, so we simply look them up
            if(var == this->numVars_-1) {
                node = this->_LookupNode(state);
            } else {
            // Non-root level variables are serialized, so we have to translate them
                node = this->_DeserializeValue(state);
            }

            if(node != nullptr) {
                sources.insert(node);
            }
        }

        if(sources.empty()) {
            return VectorType();
        } else {
            symbol_ = symbol;
            switch(val) {
                case 'X':
                    this->AscendByLevel(var, sources, dest);
                    break;
                case '1':
                    this->AscendByLevelByValue(var, 1, sources, dest);
                    break;
                case '0':
                    this->AscendByLevelByValue(var, 0, sources, dest);
                    break;
                default:
                    assert(false);
            }
            return this->CreateResultSet(dest, var != 0);
            //                                 ^--- This limits the serialization of result
        }
    }

    /**
     * @brief Wraps the MONA DFA nodes with custom layer
     *
     * @param[in]  dfa  deterministic finite automaton in MONA format // Fixme: We don't fucking need this, or not?
     */
    void ProcessDFA(DFA *dfa)
    {
        dfa_ = dfa;
        for(size_t i = this->initialState_; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));
    }

    /**
     * @brief Checks if @p node is leaf node, i.e. it has no successors.
     * Fixme: This name sucks donkey balls
     *
     * @param[in]  node  node we check if is leaf.
     */
    inline bool is_leaf(WrappedNode& node) {
        return node.pred_[0].size() == 0 && node.pred_[1].size() == 0;
    }

    /**
     * @brief Retrives final states and moreover computes some additional measures
     *
     * Retrives the MONA final states. Moreover if MEASURE_AUTOMATA_CYCLES is set,
     *   the function collectively computes several measurements regarding the cycles
     *   of the automaton on final states.
     *
     * @param[out]  final  set of final states
     */
    void GetFinalStates(VectorType& final) {
#       if (MEASURE_AUTOMATA_CYCLES == true)
        size_t universal_cycles_count = 0;
        size_t partial_cycles_count = 0;
        size_t final_states_count = 0;
#       endif

        for (size_t i = this->initialState_; i < this->dfa_->ns; ++i) {
            if (this->dfa_->f[i] == 1) {
                final.insert(i);
#               if (MEASURE_AUTOMATA_CYCLES == true)
                bool zero_cycle = false, one_cycle = false;
                if(roots_[i] != nullptr) {
                // ^---- i assume that roots_[i] means that there is no pre
                    for (auto node : roots_[i]->pred_[0]) {
                        if (is_leaf(*node) && node->node_ == i) {
                            zero_cycle = true;
                        }
                    }
                    for (auto node : roots_[i]->pred_[1]) {
                        if (is_leaf(*node) && node->node_ == i) {
                            one_cycle = true;
                        }
                    }
                }

                if(zero_cycle && one_cycle) {
                    ++universal_cycles_count;
                    ++partial_cycles_count;
                } else if (zero_cycle || one_cycle) {
                    ++partial_cycles_count;
                }
                ++final_states_count;
#               endif
            }
        }

#       if (MEASURE_AUTOMATA_CYCLES == true)
        if(universal_cycles_count == final_states_count) {
            ++MonaWrapper<Data>::noAllFinalStatesHasCycles;
            ++MonaWrapper<Data>::noSomeFinalStatesHasCycles;
        } else if(universal_cycles_count > 0) {
            ++MonaWrapper<Data>::noSomeFinalStatesHasCycles;
        }
#       endif
    }

    /**
     * @brief Returns initial state of MONA DFA
     *
     * @return initial state of MONA DFA
     */
    inline size_t GetInitialState() {
        return this->initialState_;
    }

    /**
     * @brief Returns the number of states in MONA DFA
     *
     * @return  number of states in MONA DFA
     */
    int GetMonaStateSpace() {
        return this->dfa_->ns;
    }
};

template<class Data>
int MonaWrapper<Data>::_wrapperCount = 0;
#   if (MEASURE_AUTOMATA_CYCLES == true)
template<class Data>
size_t MonaWrapper<Data>::noAllFinalStatesHasCycles = 0;    // < Number of wrapped automata, that satisfy the condition
template<class Data>
size_t MonaWrapper<Data>::noSomeFinalStatesHasCycles = 0;   // < Number of wrapped automata that satisfy this condition
template<class Data>
size_t MonaWrapper<Data>::noAllStatesHasSomeCycles = 0;     // < Number of wrapped automata that satisfy this condition
template<class Data>
size_t MonaWrapper<Data>::someStatesHasSomeCycles = 0;      // < Cumulative awerage that satisfy this condition
#   endif

template<class Data>
boost::object_pool<WrappedNode> MonaWrapper<Data>::nodePool_;

#endif // MONAWRAPPER_H
