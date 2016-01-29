#ifndef MONAWRAPPER_H
#define MONAWRAPPER_H

#include "../../Frontend/ast.h"

#include <vata/util/ord_vector.hh>

#include "ondriks_mtbdd.hh"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <assert.h>
#include <string>
#include <boost/dynamic_bitset.hpp>

template<class Data>
class MonaWrapper
{
private:
    struct WrappedNode
    {
        std::unordered_set<struct WrappedNode *> pred_[2];
        unsigned node_;
        int var_;

        WrappedNode(unsigned addr, int var = 0): node_(addr), var_(var)
        {
            ;
        }
    };

    using DataType = Data;
    using VectorType = VATA::Util::OrdVector<Data>;
    using WrappedNode = struct WrappedNode;
    using InternalNodesType = std::unordered_map<unsigned, WrappedNode *>;
    using SetType = std::unordered_set<WrappedNode *>;
    using CacheType = std::unordered_map<size_t, VectorType>;

protected:
    std::vector<WrappedNode *> roots_;
    std::vector<WrappedNode *> leafNodes_;
    InternalNodesType internalNodes_;
    DFA *dfa_;
    unsigned numVars_;
    size_t minVar_;
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

        index -= minVar_;
        if(index != BDD_LEAF_INDEX && index == 0)
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
            node.var_ = SetVar(index);
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
        return index - 1 - minVar_;
    }

    inline int SetFlag(unsigned var, bool edge)
    {
        return (var & 0xfffdffff) | (edge << 17);
    }

    inline VectorType CreateResultSet(const SetType &nodes)
    {
        VectorType vec;

        for(auto node: nodes)
            vec.insert(node->node_);

        return vec;
    }

    inline void GetSuccessors(const SetType &nodes, SetType &succs, int var, bool edge)
    {
        for(auto node: nodes)
        {
            assert(node != nullptr);
            succs.insert(node);
            if(UnequalVars(node->node_, var))
                node->var_ = SetFlag(node->var_, edge);
        }
    }

    VectorType RecPre(const SetType &nodes, int var)
    {
        SetType res;

        --var;
        if(var < 0)
        {
            return CreateResultSet(nodes);
        }

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
                    GetSuccessors(node->pred_[symbol_[(var << 1)]], res, var - 1, symbol_[(var << 1)]);
                }
                else    // don't care na vytvorenem uzlu.
                {
                    if(UnequalVars(node->var_, var - 1))
                    {
                        res.insert(node);
                    }
                    else
                    {
                        if(symbol_[(var << 1)] == ((node->var_ & 0x00020000) >> 17))
                            res.insert(node);
                    }
                }
            }
        }

        return RecPre(res, var);
    }

public:
    MonaWrapper(DFA *dfa, size_t minVar, unsigned numVars = 0): dfa_(dfa),
                                                                numVars_(numVars + minVar),
                                                                minVar_(minVar)
    {
        roots_.resize(dfa->ns, nullptr);
        leafNodes_.resize(dfa->ns, nullptr);

        for(size_t i = 1; i < dfa->ns; i++)
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

    void GetAllPathFromMona(const bdd_manager *bddm,
                            unsigned p,
                            std::string transition,
                            size_t root,
                            size_t varNum)
    {
        unsigned l, r, index;

        LOAD_lri(&bddm->node_table[p], l, r, index);

        if (index == BDD_LEAF_INDEX)
        {
            std::cout << root << " -(" << transition << ")-> " << l << std::endl;
        }
        else
        {
            transition[index] = '0';
            GetAllPathFromMona(bddm, l, transition, root, varNum);

            transition[index] = '1';
            GetAllPathFromMona(bddm, r, transition, root, varNum);
        }
    }

    void DumpDFA() {
        std::string str(this->numVars_, 'X');
        for(int i = 1; i < this->dfa_->ns; ++i) {
            GetAllPathFromMona(this->dfa_->bddm, this->dfa_->q[i], str, i, this->numVars_);
        }
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
        for(size_t i = 1; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));
    }

    void GetFinalStates(VectorType& final) {
        for (unsigned int i = 1; i < this->dfa_->ns; ++i) {
            if (this->dfa_->f[i] == 1) {
                final.insert(i);
            }
        }
    }
};


#endif // MONAWRAPPER_H
