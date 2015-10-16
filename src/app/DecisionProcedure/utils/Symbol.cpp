//
// Created by Raph on 09/10/2015.
//

#include "Symbol.h"

extern VarToTrackMap varMap;

char ZeroSymbol::charToAsgn(char c) {
    switch(c) {
        case '0':
            return 0x01;
            break;
        case '1':
            return 0x02;
            break;
        case 'X':
            return 0x03;
            break;
        default:
            assert(false);
    }
}

BaseAut_MTBDD* ZeroSymbol::GetMTBDD() {
    // TODO: OPTIMIZE
    /*if(this->_bdd == nullptr) {
        this->_bdd = new BaseAut_MTBDD(this->_track, BaseAut_States(StateTuple({0})), BaseAut_States(StateTuple({})));
    }
    assert(this->_bdd != nullptr);
    return this->_bdd;*/
    return new BaseAut_MTBDD(this->_track, BaseAut_States(StateTuple({0})), BaseAut_States(StateTuple({})));
}

Automaton::SymbolType ZeroSymbol::constructUniversalTrack() {
    unsigned int trackLen = varMap.TrackLength() - 1;
    Automaton::SymbolType transitionTrack;
    transitionTrack.AddVariablesUpTo(trackLen);
    return transitionTrack;
}

Automaton::SymbolType ZeroSymbol::constructZeroTrack() {
    unsigned int trackLen = varMap.TrackLength();
    std::string track(trackLen, '0');
    return Automaton::SymbolType(track.c_str());
}

void ZeroSymbol::ProjectVar(Var var) {
    assert(this->_bdd == nullptr);
    this->_track.SetIthVariableValue(var, charToAsgn('X'));
}

void ZeroSymbol::ProjectVars(Vars freeVars) {
    assert(this->_bdd == nullptr);
    for(auto var : freeVars) {
        this->_track.SetIthVariableValue(var, charToAsgn('X'));
    }
}

ZeroSymbol::ZeroSymbol() {
    this->_track = ZeroSymbol::constructZeroTrack();
    this->_bdd = nullptr;
}

ZeroSymbol::ZeroSymbol(Automaton::SymbolType track) {
    this->_track = track;
    this->_bdd = nullptr;
}

ZeroSymbol::ZeroSymbol(Automaton::SymbolType track, Var var, Value val) {
    this->_track = track;
    if(this->_track.GetIthVariableValue(var) != charToAsgn('X')) {
        this->_track.SetIthVariableValue(var, charToAsgn(val));
    }
    this->_bdd = nullptr;
}

bool ZeroSymbol::IsEmpty() {
    return this->_track.length() == 0;
}

std::ostream& operator <<(std::ostream& osObject, const ZeroSymbol& z) {
    osObject << z._track.ToString();
    //osObject << BaseAut_MTBDD::DumpToDot({z._bdd}) << "\n";
    return osObject;
}