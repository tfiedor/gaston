//
// Created by Raph on 27/01/2016.
//

#include <iostream>
#include "MonaAutomataDotWalker.h"
#include "DotWalker.h"
#include "../../../Frontend/dfa.h"
#include "../../mtbdd/monawrapper.hh"
#include "../../containers/VarToTrackMap.hh"
#include "../../automata.hh"
#include "../../environment.hh"

extern VarToTrackMap varMap;

TimeType MonaAutomataDotWalker::_constructAutomaton(ASTForm* form) {
    DFA *monaAutomaton = nullptr;
    try {
        DFA *temp = nullptr;
        size_t monaStates = 0, minimizedStates = 0;
        toMonaAutomaton(form, monaAutomaton, false);
        assert(monaAutomaton != nullptr);
        monaStates = monaAutomaton->ns;

        temp = dfaCopy(monaAutomaton);
        if(_print_intermediate && monaStates >= this->_threshold) {
            std::ofstream pre_minimization;
            std::string pm_file(this->_fileName);
            pm_file += "_" + std::to_string(this->_counter) + "__" + std::to_string(monaStates) + "states.dfa";
            pre_minimization.open(pm_file);
            pre_minimization << "(*" << form->ToString(true) << "*)\n";
            MonaWrapper<size_t> wrapper(monaAutomaton, true, varMap.TrackLength());
            wrapper.DumpTo(pre_minimization);

        }

        dfaUnrestrict(temp);
        monaAutomaton = dfaMinimize(temp);
        minimizedStates = monaAutomaton->ns;

        temp = dfaCopy(monaAutomaton);
        if(_print_intermediate && monaStates >= this->_threshold) {
            std::ofstream post_minimization;
            std::string am_file(this->_fileName);
            am_file += "_" + std::to_string(this->_counter++) + "__" + std::to_string(minimizedStates) + "states.mdfa";
            post_minimization.open(am_file);
            post_minimization << "(*" << form->ToString(true) << "*)\n";
            MonaWrapper<size_t> wrapper(temp, true, varMap.TrackLength());
            wrapper.DumpTo(post_minimization);
        }

        // Clean up
        dfaFree(monaAutomaton);
        monaAutomaton = nullptr;
        /*dfaFree(temp);
        temp = nullptr;*/
        return std::make_pair(minimizedStates, monaStates);
    } catch (std::exception &e) {
        if(monaAutomaton != nullptr) {
            dfaFree(monaAutomaton);
        }
        return std::make_pair(0, 0);
    }
}

void MonaAutomataDotWalker::visit(ASTForm_And* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u2227\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void MonaAutomataDotWalker::visit(ASTForm_Or* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u2228\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void MonaAutomataDotWalker::visit(ASTForm_Impl* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u21D2\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void MonaAutomataDotWalker::visit(ASTForm_Biimpl* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u21D4\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void MonaAutomataDotWalker::visit(ASTForm_Not* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u00AC\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f << ";\n";
}

template<class ExistClass>
void MonaAutomataDotWalker::_existsToDot(ExistClass* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u2203";
    for(auto it = form->vl->begin(); it != form->vl->end(); ++it) {
        this->_dotFile << (*it) << ", ";
    }
    this->_dotFile << "\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f << ";\n";
}

void MonaAutomataDotWalker::visit(ASTForm_Ex1* form) {
    this->_existsToDot<ASTForm_Ex1>(form);
}
void MonaAutomataDotWalker::visit(ASTForm_Ex2* form) {
    this->_existsToDot<ASTForm_Ex2>(form);
}

template<class ForallClass>
void MonaAutomataDotWalker::_forallToDot(ForallClass* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] \u2200";
    for(auto it = form->vl->begin(); it != form->vl->end(); ++it) {
        this->_dotFile << (*it) << ", ";
    }
    this->_dotFile << "\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f << ";\n";
}

void MonaAutomataDotWalker::visit(ASTForm_All1* form) {
    this->_forallToDot<ASTForm_All1>(form);
}

void MonaAutomataDotWalker::visit(ASTForm_All2* form) {
    this->_forallToDot<ASTForm_All2>(form);
}

void MonaAutomataDotWalker::_atomicToDot(ASTForm* form) {
    TimeType automatonSize = this->_constructAutomaton(form);
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"[" << (std::to_string(form->tag)) << "] " << form->ToString() << "\\n" << std::to_string(automatonSize.first) << "/" << std::to_string(automatonSize.second) << "\"];\n";
}


void MonaAutomataDotWalker::visit(ASTForm_True* form) {
    this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_False* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_In* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_Notin* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_RootPred* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_EmptyPred* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_FirstOrder* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_Sub* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_Equal1* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_Equal2* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_NotEqual1* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_NotEqual2* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_Less* form) {
	this->_atomicToDot(form);
}

void MonaAutomataDotWalker::visit(ASTForm_LessEq* form) {
	this->_atomicToDot(form);
}
