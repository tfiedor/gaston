#ifndef MONAWRAPPER_H
#define MONAWRAPPER_H

#include "../../Frontend/ast.h"

#include <vata/util/ord_vector.hh>

#include "ondriks_mtbdd.hh"
#include "../containers/VarToTrackMap.hh"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <assert.h>
#include <string>
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>

extern VarToTrackMap varMap;

template<class Data>
class MonaWrapper
{
private:
    template<class T>
    struct WrappedCompare;

    struct WrappedNode
    {

        std::unordered_set<struct WrappedNode*, boost::hash<struct WrappedNode*>, WrappedCompare<struct WrappedNode*>> pred_[2];
        unsigned node_;
        int var_;

        WrappedNode(unsigned addr, int var = 0): node_(addr), var_(var)
        {
            ;
        }
    };

    template<class Key>
    struct WrappedCompare : public std::binary_function<Key, Key, bool> {
        bool operator()(Key const& lhs, Key const& rhs) const {
            return lhs->node_ == rhs->node_ && lhs->var_ == rhs->var_;
        }
    };

    using DataType = Data;
    using VectorType = VATA::Util::OrdVector<Data>;
    using WrappedNode = struct WrappedNode;
    using InternalNodesType = std::unordered_map<unsigned, WrappedNode *>;
    using HashType = boost::hash<WrappedNode*>;
    using SetType = std::unordered_set<WrappedNode*, HashType, WrappedCompare<WrappedNode*>>;
    using CacheType = std::unordered_map<size_t, VectorType>;

protected:
    std::vector<WrappedNode*> roots_;
    std::vector<WrappedNode*> leafNodes_;
    InternalNodesType internalNodes_;
    DFA *dfa_;
    unsigned numVars_;
    size_t initialState_;
    boost::dynamic_bitset<> symbol_;

private:
    inline WrappedNode *spawnNode(unsigned addr, WrappedNode &pred, bool edge)
    {
        WrappedNode *node;
        typename InternalNodesType::iterator itN;

        if((itN = internalNodes_.find(addr)) != internalNodes_.end())
        {
            node = itN->second;
        }
        else
        {
            node = new WrappedNode(addr);
            internalNodes_.insert(std::make_pair(addr, node));
        }

        node->pred_[edge].insert(&pred);
        return node;
    }

    inline WrappedNode *spawnNode(const bdd_manager *bddm, unsigned addr, size_t state)
    {
        unsigned index;

        leafNodes_[state] = new WrappedNode(state, 0xfffffffe);
        LOAD_index(&bddm->node_table[addr], index);

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

    VectorType RecPre(const SetType &nodes, int var)
    {
        --var;
        if(var < 0)
        {
            return CreateResultSet(nodes);
        }

        SetType res;

        if(symbol_[var << 1] & symbol_[(var << 1) + 1])
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
            // neni don't care.
            for(auto node: nodes)
            {
                assert(node != nullptr);
                if(GetVar(node->var_) == var)
                {
                    GetDontCareSucc(node->pred_[~symbol_[(var << 1)]], res, var - 1, ~symbol_[(var << 1)]);
                    GetSuccessors(node->pred_[symbol_[(var << 1)]], res, var - 1, symbol_[(var << 1)]);
                    ResetFlags(node->var_);
                }
                else    // don't care na vytvorenem uzlu.
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

        return RecPre(res, var);
    }

public:
    MonaWrapper(DFA *dfa, bool emptyTracks, unsigned numVars = 0): dfa_(dfa), numVars_(numVars), initialState_(emptyTracks ? 0 : 1)
    {
        roots_.resize(dfa->ns, nullptr);
        leafNodes_.resize(dfa->ns, nullptr);

        for(size_t i = this->initialState_; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));
    }

    ~MonaWrapper()
    {
        for(auto node: internalNodes_)
            delete node.second;

        for(auto leaf: leafNodes_)
            delete leaf;
        dfaFree(this->dfa_);
    }

    void DumpToDot()
    {
        /*std::ofstream ofs ("bu.gv", std::ofstream::out);

        ofs << "digraph M {" << std::endl;

        ofs << "subgraph cluster0 {" << std::endl;
        ofs << "color=\"#008000\";" << std::endl;
        for(size_t i = 1; i < roots_.size(); i++)
            ofs << "s" << i << ";" << std::endl;
        ofs << "}" << std::endl;

        ofs << "subgraph cluster1 {" << std::endl;
        ofs << "color=\"#800000\";" << std::endl;
        for(auto leaf: leafNodes_)
            ofs << "t" << leaf->node_ << "[label=\"" << leaf->node_ << " (" << GetVar(leaf->var_) << ")\"];" << std::endl;
        ofs << "}" << std::endl;

        for(auto node: internalNodes_)
            ofs << node.first << "[label=\"" << node.first << " (" << GetVar(node.second->var_) << ")\"];" << std::endl;

        for(size_t i = 1; i < roots_.size(); i++)
            ofs << "s" << i << " -> " << roots_[i]->node_ << ";" << std::endl;

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

        ofs << "}" << std::endl;*/
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

        if(roots_[state] == nullptr)
            return VectorType();

        symbol_ = symbol;
        return RecPre({roots_[state]}, numVars_);
    }

    void ProcessDFA(DFA *dfa)
    {
        dfa_ = dfa;
        for(size_t i = this->initialState_; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));
    }

    void GetFinalStates(VectorType& final) {
        for (size_t i = this->initialState_; i < this->dfa_->ns; ++i) {
            if (this->dfa_->f[i] == 1) {
                final.insert(i);
            }
        }
    }

    size_t GetInitialState() {
        return this->initialState_;
    }
};


#endif // MONAWRAPPER_H
