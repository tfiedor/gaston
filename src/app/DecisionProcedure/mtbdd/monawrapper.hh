#ifndef MONAWRAPPER_H
#define MONAWRAPPER_H

#include "../../Frontend/ast.h"
#include "../containers/SymbolicCache.hh"

#include <vata/util/ord_vector.hh>

#include "ondriks_mtbdd.hh"
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
    //using CacheType         = std::unordered_map<size_t, VectorType>;
    //using PreCache  = BinaryCache<PreKey, Term_ptr, PreHashType, PrePairCompare<PreKey>, dumpPreKey, dumpPreData>;

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
    static int _wrapperCount;
#   if (MEASURE_AUTOMATA_CYCLES == true)
    static size_t noAllFinalStatesHasCycles;    // < Number of wrapped automata, that satisfy the condition
    static size_t noSomeFinalStatesHasCycles;   // < Number of wrapped automata that satisfy this condition
    static size_t noAllStatesHasSomeCycles;     // < Number of wrapped automata that satisfy this condition
    static size_t someStatesHasSomeCycles;      // < Cumulative awerage that satisfy this condition
#   endif

private:
    WrappedNode *spawnNode(unsigned addr, WrappedNode &pred, bool edge)
    {
        WrappedNode *node;
        typename InternalNodesType::iterator itN;

        if((itN = internalNodes_.find(addr)) != internalNodes_.end())
        {
            node = itN->second;
        }
        else
        {
            node = MonaWrapper<Data>::nodePool_.construct(addr);
            //node = new WrappedNode(addr);
            internalNodes_.insert(std::make_pair(addr, node));
        }

        node->pred_[edge].insert(&pred);
        return node;
    }

    WrappedNode *spawnNode(const bdd_manager *bddm, unsigned addr, size_t state)
    {
        unsigned index;

        leafNodes_[state] = MonaWrapper<Data>::nodePool_.construct(state,  0xfffffffe);
        //leafNodes_[state] = new WrappedNode(state, 0xfffffffe);
        LOAD_index(&bddm->node_table[addr], index);

        // Fixme: The fuck is this cock magic?
        if(index != BDD_LEAF_INDEX && varMap[index] == 0)
            return leafNodes_[state];

        WrappedNode *result = spawnNode(addr, *leafNodes_[state], 1);
        result->pred_[0].insert(leafNodes_[state]);
        return result;
    }

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

    inline int GetVar(int var)
    {
        return var & 0x1ffff;
    }

    inline bool UnequalVars(int var1, int var2)
    {
        return GetVar(var1) ^ GetVar(var2);
    }

    inline int SetVar(unsigned index)
    {
        return (index - 1) | 0x00060000;
    }

    inline void SetFlag(int &var, bool edge)
    {
        var = var & (0xfffbffff >> edge);
    }

    inline bool IsPredecessorHigh(int var)
    {
        return (var & 0x00020000) ^ 0x00020000;
    }

    inline bool IsPredecessorLow(int var)
    {
        return (var & 0x00040000) ^ 0x00040000;
    }

    inline void ResetFlags(int &var)
    {
        var = var | 0x00060000;
    }

    inline VectorType CreateResultSet(const SetType &nodes)
    {
        VectorType vec;

        for(auto node: nodes)
            vec.insert(node->node_);

        return vec;
    }

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

    inline Gaston::BitMask transformSymbol(const Gaston::BitMask &symbol, int var)
    {
        Gaston::BitMask transformed = symbol;
        transformed &= symbolMasks_[var];
        transformed |= masks_[var];
        return transformed;
//        return (symbolMasks_[var] & symbol) | masks_[var];
    }

    void RecPre(WrappedNode *node, VectorType &res)
    {
        //std::cout << "node->var_: " << node->var_ << std::endl;
        if(node->var_ < 0)
        {
            res.insert(node->node_);
            return;
        }

        int var = GetVar(node->var_);

        // Vytvorit key, ktery bude dvojice (stav, podcesta)
        // => vytvori se funkce, ktera toto vytvori.
        // potom, je treba vysledek sjednotit s jiz existujicimi vysledky.
#       if (OPT_CACHE_SUBPATHS_IN_WRAPPER == true)
        VectorType tmp;
        auto key = std::make_pair(node, transformSymbol(symbol_, var));

        if(cache_.retrieveFromCache(key, tmp))
        {
            res.insert(tmp);
            return;
        }
#       endif

        //std::cout << "pokus" << std::endl;
        VectorType innerRes;
        if(/*symbol_[var << 1] &&*/ symbol_[(var << 1) + 1])
        {
            for(auto nnode: node->pred_[~symbol_[(var << 1)]])
            {
                if(UnequalVars(nnode->var_, var - 1) && nnode->var_ > -2)
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
                if(UnequalVars(nnode->var_, var - 1) && nnode->var_ > -2)
                    if((~symbol_[(var << 1)] == symbol_[(GetVar(nnode->var_ + 1) << 1)]) ||
                       symbol_[(GetVar(nnode->var_ + 1) << 1) + 1])
                        RecPre(nnode, innerRes);
        }

        for(auto nnode: node->pred_[symbol_[(var << 1)]])
        {
            if(UnequalVars(nnode->var_, var - 1) && nnode->var_ > -2)
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

    VectorType RecPre(SetType& nodes)
    {
        //SetType nodes({root});
        SetType res;

        int var = numVars_ - 1;
        for(; -1 < var; --var)
        {
            if(/*symbol_[var << 1] &*/ symbol_[(var << 1) + 1])
            // ^-- this is excessive, since only testing 2n+1 bit is enough to determine if don't care
            {
                // don't care.
                for(auto node: nodes)
                {
                    assert(node != nullptr);
                    if(UnequalVars(node->var_, var))
                    {
                        res.insert(node);
                    }
                    else
                    {
                        GetSuccessors(node->pred_[0], res, var - 1, 0);
                        GetSuccessors(node->pred_[1], res, var - 1, 1);
                        ResetFlags(node->var_);
                    }
                }
            }
            else
            {
                // not "don't care".
                for(auto node: nodes)
                {
                    assert(node != nullptr);
                    if(GetVar(node->var_) == var)
                    {
                        GetDontCareSucc(node->pred_[~symbol_[(var << 1)]], res, var - 1, ~symbol_[(var << 1)]);
                        GetSuccessors(node->pred_[symbol_[(var << 1)]], res, var - 1, symbol_[(var << 1)]);
                        ResetFlags(node->var_);
                    }
                    else    // don't care on(at?) created node.
                    {
                        if(UnequalVars(node->var_, var - 1))
                        {
                            res.insert(node);
                        }
                        else
                        {
                            if((symbol_[(var << 1)] && IsPredecessorHigh(node->var_)) ||
                               (~symbol_[(var << 1)] && IsPredecessorLow(node->var_)))
                                res.insert(node);

                            ResetFlags(node->var_);
                        }
                    }
                }
            }

            nodes = std::move(res);
        }

        return CreateResultSet(nodes);
    }

public:
    MonaWrapper(DFA *dfa, bool emptyTracks, bool is_restriction, unsigned numVars = 0)
            : dfa_(dfa), numVars_(numVars), initialState_(emptyTracks ? 0 : 1), isRestriction_(is_restriction)
    {
        roots_.resize(dfa->ns, nullptr);
        leafNodes_.resize(dfa->ns, nullptr);

        //std::cout << "Velikost mony je " << dfa->ns << std::endl;

        mask_.resize(numVars_ << 1);
        for(unsigned i = 0; i < numVars_; ++i)
            mask_[(i << 1) + 1] = true;

        Gaston::BitMask symbolMask;
        symbolMask.resize(numVars_ << 1);
        symbolMask.set();

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
#           if (PRINT_IN_TIMBUK == true)
            os << transition << "(" << root << ") -> " << l << "\n";
#           else
            os << root << " -(" << transition << ")-> " << l << "\n";
#           endif
        }
        else
        {
            transition[varMap[index]] = '0';
            GetAllPathFromMona(os, bddm, l, transition, root, varNum);

            transition[varMap[index]] = '1';
            GetAllPathFromMona(os, bddm, r, transition, root, varNum);
        }
    }

    void DumpDFA(std::ostream &os) {
        std::string str(this->numVars_, 'X');
        for(size_t i = this->initialState_; i < this->dfa_->ns; ++i) {
            GetAllPathFromMona(os, this->dfa_->bddm, this->dfa_->q[i], str, i, this->numVars_);
        }
    }

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

    VectorType Pre(size_t state, const boost::dynamic_bitset<> &symbol)
    {
        assert(dfa_ != nullptr);
        assert(roots_.size() > state);

        //std::cout << "===================================================" << std::endl << std::endl << std::endl;
        if(roots_[state] == nullptr)
            return VectorType();

        symbol_ = symbol;
        //exploredState_ = state;

        VectorType res;
        RecPre(roots_[state], res);

        //std::cout << std::endl << std::endl << std::endl << "===================================================" << std::endl;
        return res;
    }

    VectorType Pre(VATA::Util::OrdVector<size_t> &states, const boost::dynamic_bitset<> &symbol)
    {
        assert(dfa_ != nullptr);
        SetType nodes;

        for(auto state: states)
        {
            assert(roots_.size() > state);
            if(roots_[state] != nullptr)
                nodes.insert(roots_[state]);
        }

        if(nodes.empty())
            return VectorType();

        symbol_ = symbol;
        return RecPre(nodes);
    }

    void ProcessDFA(DFA *dfa)
    {
        dfa_ = dfa;
        for(size_t i = this->initialState_; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));
    }

    inline bool is_leaf(WrappedNode& node) {
        return node.pred_[0].size() == 0 && node.pred_[1].size() == 0;
    }

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

    inline size_t GetInitialState() {
        return this->initialState_;
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
