#ifndef MTBDDCONVERTER_H
#define MTBDDCONVERTER_H

#include "WrappedUnorderedSet.hh"

#include "../../Frontend/ast.h"

#include "ondriks_mtbdd.hh"
//#include "../MTBDDConverter/mtbddconverter.hh"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <assert.h>
#include <string>
#include <fstream>
#include <boost/dynamic_bitset/config.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>

template <class TDataType>
class MTBDDConverter2
{
private:
    struct WrappedNode
    {
        std::vector<struct WrappedNode *> pred_[2];
        unsigned node_;
        int var_;

        WrappedNode(unsigned addr, int var = 0): node_(addr), var_(var)
        {
            ;
        }
    };

    using DataType = TDataType;
    using WrappedNode = struct WrappedNode;
    using InternalNodesType = std::unordered_map<unsigned, WrappedNode *>;
    using NodePtrType = VATA::MTBDDPkg::MTBDDNodePtr<DataType>;
    using MTBDD = VATA::MTBDDPkg::OndriksMTBDD<DataType>;
    //using SetType = std::unordered_set<WrappedNode *>;
    using SetType = WrappedUnorderedSet<WrappedNode *>;
    using CacheType = std::unordered_map<SetType, NodePtrType, boost::hash<SetType>>;

private:
    std::vector<WrappedNode *> roots_;
    std::vector<WrappedNode *> leafNodes_;
    std::vector<WrappedNode *> editNodes_;
    InternalNodesType internalNodes_;
    NodePtrType defaultNode_;
    CacheType cache_;
    unsigned numVars_;

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

        node->pred_[edge].push_back(&pred);
        return node;
    }

    inline WrappedNode *spawnNode(const bdd_manager *bddm, unsigned addr, size_t state)
    {
        unsigned index;

        leafNodes_[state] = new WrappedNode(state | 0x01000000, 0xfffffffe);
        LOAD_index(&bddm->node_table[addr], index);

        if(index != BDD_LEAF_INDEX && index == 0)
            return leafNodes_[state];

        WrappedNode *result = spawnNode(addr, *leafNodes_[state], 1);
        //result->pred_[0].push_back(leafNodes_[state]);
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

    inline NodePtrType CreateVATALeaf(const SetType &nodes)
    {
        DataType data;

        for(auto node: nodes)
            data.insert(GetAddr(node->node_));

        return MTBDD::spawnLeaf(data);
    }


    inline void SetFlag(int &var, bool edge)
    {
        var = var & (0xfffbffff >> edge);
    }

    inline void ResetFlags(int &var)
    {
        var = var | 0x00060000;
    }

    inline void getSuccessors(SetType &nodes1,
                              SetType &nodes2,
                              const std::vector<WrappedNode *> &pred,
                              int var,
                              bool edge)
    {
        for(auto lnode: pred)
        {
            //nodes1.insert(lnode);
            nodes1.insert(lnode, var);
            if(UnequalVars(lnode->var_, var))
            {
                SetFlag(lnode->var_, edge);
                editNodes_.push_back(lnode);
                nodes2.insert(lnode, var);
                //nodes2.insert(lnode);
            }
        }
    }

    inline unsigned GetVar(int var)
    {
        return var & 0x1ffff;
    }

    inline unsigned GetAddr(unsigned addr)
    {
        return addr & 0x00ffffff;
    }

    inline bool UnequalVars(int var1, int var2)
    {
        return GetVar(var1) ^ GetVar(var2);
    }

    inline int SetVar(unsigned index)
    {
        return (index - 1) | 0x00060000;
    }

    inline bool IsPredecessorHigh(int var)
    {
        return (var & 0x00020000) ^ 0x00020000;
    }

    inline bool IsPredecessorLow(int var)
    {
        return (var & 0x00040000) ^ 0x00040000;
    }

    // smazat
    void setSpaces(std::string &str, size_t len)
    {
        for(; len > str.size(); str += " ");
    }

    // smazat
    void printDebugInfo(const SetType &nodes, const std::string &str, int var)
    {
        std::string spaces;

        setSpaces(spaces, (numVars_ - 1 - var) << 2);
        std::cout << spaces << str;
        for(auto node: nodes)
            std::cout << GetAddr(node->node_) << " ";
        std::cout << std::endl;
    }

    void printDebugInfo(const std::string &str, int var)
    {
        std::string spaces;

        setSpaces(spaces, (numVars_ - 1 - var) << 2);
        std::cout << spaces << str << var << std::endl;
    }

    void printDebugInfo(const std::string &str, NodePtrType ptr, int var)
    {
        std::string spaces;

        setSpaces(spaces, (numVars_ - 1 - var) << 2);
        std::cout << spaces << str << ptr << std::endl;
    }

    /*inline void spawnSet(SetType &set)
    {
        typename CacheType::const_iterator itC;
        if ((itC = cache_.find(set)) != cache_.end())
        {	// in case given internal is already cached
            result = itC->second;
        }
        else
        {	// if the internal doesn't exist
            result = CreateInternal(low, high, var);
            IncrementRefCnt(low);
            IncrementRefCnt(high);
            internalCache_.insert(std::make_pair(addr, result));
        }

        assert(!IsNull(result));
        return result;
    }*/

    NodePtrType CreateVATAMTBDD(const SetType &nodes, int var)
    {
        SetType highs;
        SetType lows;
        NodePtrType low = defaultNode_;
        NodePtrType high =  defaultNode_;

        --var;
        if(var < 0)
            return CreateVATALeaf(nodes);

        //std::cout << " hash_value(nodes): " << hash_value(nodes) << std::endl;
        typename CacheType::const_iterator it;
        if((it = cache_.find(nodes)) != cache_.end())
            return it->second;

        for(auto node: nodes)
        {
            if(UnequalVars(node->var_, var))
            { // Don't care.
                if(UnequalVars(node->var_, var - 1))
                {
                    highs.insert(node);
                    lows.insert(node);
                }
                else
                {
                    if(IsPredecessorLow(node->var_))
                        lows.insert(node);

                    if(IsPredecessorHigh(node->var_))
                        highs.insert(node);
                }
            }
            else
            {
                getSuccessors(lows, highs, node->pred_[0], var - 1, 0);
                getSuccessors(highs, lows, node->pred_[1], var - 1, 1);
            }
        }

        //printDebugInfo("var: ", var);
        if(lows.size())
        {
            //printDebugInfo(lows, "low: ", var);
            low = CreateVATAMTBDD(lows, var);
            //printDebugInfo("addr: ", low, var);
        }

        if(highs.size())
        {
            //printDebugInfo(highs, "high: ", var);
            high = CreateVATAMTBDD(highs, var);
            //printDebugInfo("addr: ", high, var);
        }

        if(low == high)
        {
            cache_.insert(std::make_pair(nodes, low));
            return low;
        }

        /*std::cout << "var: " << var << std::endl;
        std::cout << "   high: ";
        for(auto hnode: highs)
            std::cout << hnode->node_ << " ";
        std::cout << " (" << high << ")" << std::endl;

        std::cout << "    low: ";
        for(auto lnode: lows)
            std::cout << lnode->node_ << " ";
        std::cout << " (" << low << ")" << std::endl;

        std::cout << " hash_value(lows): " << hash_value(lows) << std::endl;
        std::cout << "hash_value(highs): " << hash_value(highs) << std::endl;*/


        if(highs.size() || lows.size())
        {
            NodePtrType result = MTBDD::spawnInternal(low, high, var);
            cache_.insert(std::make_pair(nodes, result));
            return result;
        }
        else
        {
            return CreateVATAMTBDD(nodes, var);
        }
    }


public:
    MTBDDConverter2(size_t stateNum, unsigned numVars = 0): numVars_(numVars),
                                                            defaultNode_(MTBDD::spawnLeaf(DataType()))
    {
        roots_.resize(stateNum);
        leafNodes_.resize(stateNum);

        //std::hash<std::unordered_set<WrappedNode *>> str_hash;
        //std::unordered_map<std::unordered_set<WrappedNode *>, NodePtrType> pokus;
        //std::unordered_map<DataType, NodePtrType> pokus;
    }

    ~MTBDDConverter2()
    {
        for(auto node: internalNodes_)
            delete node.second;

        for(auto leaf: leafNodes_)
            delete leaf;
    }

    void DumpToDot(DFA *dfa)
    {
        std::ofstream ofs ("bu.gv", std::ofstream::out);

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

        ofs << "}" << std::endl;
    }

    void Process(DFA *dfa, Automaton& v_aut)
    {
        for(size_t i = 0; i < dfa->ns; i++)
            RecSetPointer(dfa->bddm, dfa->q[i], *spawnNode(dfa->bddm, dfa->q[i], i));

        //DumpToDot(dfa);
        for(size_t i = 1; i < roots_.size(); i++)
        {
            //std::cout << CreateVATAMTBDD({roots_[i]}, numVars_) << std::endl;
            NodePtrType root = CreateVATAMTBDD({roots_[i]}, numVars_);
            VATA::MTBDDPkg::IncrementRefCnt(root);
            v_aut.SetMtbdd({i}, MTBDD(root, {}));
            for(auto node: editNodes_)
                ResetFlags(node->var_);

            editNodes_.clear();
            //std::cout << "====================================" << std::endl;
        }

        //MTBDDConverter::VATADumpToDot();
    }

    void SetMTBDDToVATA(Automaton &v_aut)
    {
        /*for(size_t i = 1; i < roots_.size(); i++)
            v_aut.SetMtbdd({i}, MTBDD(roots_[i]->node_, defaultNode_));*/
    }
};

#endif // MTBDDCONVERTER_H
