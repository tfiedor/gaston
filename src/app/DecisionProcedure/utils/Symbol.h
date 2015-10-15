//
// Created by Raph on 09/10/2015.
//

#ifndef WSKS_SYMBOL_H
#define WSKS_SYMBOL_H

#include <vata/bdd_bu_tree_aut.hh>
#include "../containers/VarToTrackMap.hh"
#include "../mtbdd/ondriks_mtbdd.hh"

using Automaton = VATA::BDDBottomUpTreeAut;
using StateType = size_t;
using StateTuple = std::vector<StateType>;
using Value = char;
using Var = size_t;
using Vars = std::vector<Var>;
using BaseAut_States = VATA::Util::OrdVector<StateType>;
using BaseAut_MTBDD = VATA::MTBDDPkg::OndriksMTBDD<BaseAut_States>;

class ZeroSymbol {
public:

private:
    Automaton::SymbolType _track;
    BaseAut_MTBDD* _bdd;

public:
    ZeroSymbol();
    ZeroSymbol(Automaton::SymbolType);
    ZeroSymbol(Automaton::SymbolType, Var, Value);

    void ProjectVars(Vars freeVars);
    void ProjectVar(Var var);
    bool IsEmpty();
    Automaton::SymbolType GetTrack() { return this->_track; }
    BaseAut_MTBDD* GetMTBDD();

    static Automaton::SymbolType constructUniversalTrack();
    static Automaton::SymbolType constructZeroTrack();
    static char charToAsgn(char c);
    friend std::ostream& operator <<(std::ostream& osObject, const ZeroSymbol& z);
};

#endif //WSKS_SYMBOL_H
