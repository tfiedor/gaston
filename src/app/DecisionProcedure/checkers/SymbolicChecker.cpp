//
// Created by Raph on 02/02/2016.
//

#include <stdio.h>
#include <ostream>
#include <iostream>
#include <sstream>
#include <csignal>
#include "SymbolicChecker.h"
#include "../containers/Term.h"
#include "../containers/Workshops.h"
#include "../environment.hh"
#include "../../Frontend/timer.h"
#include "../../Frontend/env.h"
#include "../visitors/transformers/Derestricter.h"

extern Timer timer_conversion, timer_mona, timer_base, timer_automaton, timer_preprocess, timer_closure, timer_parse;
extern Ident lastPosVar, allPosVar;
extern Options options;
extern char *inputFileName;

SymbolicChecker::~SymbolicChecker() {

}

/**
 * Construction of Automaton for Symbolic Decision procedure
 */
void SymbolicChecker::ConstructAutomaton() {
    assert(this->_monaAST != nullptr);

    timer_automaton.start();
    this->_automaton = (this->_monaAST->formula)->toSymbolicAutomaton(false);

    if(allPosVar != -1) {
        std::cout << "[*] AllPosVar predicate detected. Will use the M2L(str) decision procedure.\n";
    }

    // Formulae with free variable will be decided using the backward search, that is implemented in the RootProjection
    IdentList free, bound;
    this->_monaAST->formula->freeVars(&free, &bound);
    this->_beforeClosure = this->_monaAST->formula;
    if (!free.empty()) {
        if(this->_rootRestriction != nullptr && this->_rootRestriction->kind != aTrue) {
            this->_monaAST->formula = new ASTForm_And(this->_rootRestriction, this->_monaAST->formula, Pos());
            SymbolicAutomaton* restrAutomaton = this->_rootRestriction->toSymbolicAutomaton(false);
            restrAutomaton->MarkAsRestriction();
            this->_automaton = new IntersectionAutomaton(restrAutomaton, this->_automaton, this->_monaAST->formula);
        }
        this->_monaAST->formula = new ASTForm_Ex1(nullptr, free.copy(), this->_monaAST->formula, Pos());
        //                        ^---- this is just a placeholding Ex1, semantically it is not First Order
        this->_automaton = new RootProjectionAutomaton(this->_automaton, this->_monaAST->formula);
    }

    if(options.printProgress && !options.dontDumpAutomaton)
        this->_automaton->DumpAutomaton();
    std::cout << "\n";
#   if (MEASURE_AUTOMATA_METRICS == true)
    this->_automaton->DumpAutomatonMetrics();
#   endif

    timer_automaton.stop();
    if (options.dump) {
        std::cout << "\n[*] Formula translation to Automaton [DONE]\n";
        std::cout << "[*] Elapsed time: ";
        timer_automaton.print();
        std::cout << "\n";
    }
}

/**
 * Core evaluation of decision procedure according to the membership testing of epsilon and/or the found (counter)
 * examples during the backward search
 *
 * @param[in] isValid:  true if the epsilon is in the language of the automaton
 */
Decision SymbolicChecker::_DecideCore(bool isValid) {
    assert(this->_automaton != nullptr);

    if (this->_terminatedBySignal) {
        return Decision::UNKNOWN;
    } else if(allPosVar != -1 && options.test != TestType::EVERYTHING) {
        if(options.test == TestType::VALIDITY) {
            return (this->_automaton->_unsatExample == nullptr) ? Decision::VALID : Decision::INVALID;
        } else if(options.test == TestType::SATISFIABILITY) {
            return (this->_automaton->_satExample != nullptr) ? Decision::SATISFIABLE : Decision::UNSATISFIABLE;
        } else {
            assert(options.test == TestType::UNSATISFIABILITY);
            return (this->_automaton->_satExample != nullptr) ? Decision::UNSATISFIABLE : Decision::SATISFIABLE;
        }
    } else if(this->_isGround) {
        // Ground formula is valid if epsilon is in its language
        if (isValid) {
            return Decision::VALID;
            // Else it is unsatisfiable
        } else {
            return Decision::UNSATISFIABLE;
        }
    } else {
        assert(options.test == TestType::EVERYTHING);
        // Formula is unsatisfiable if there is no satisfying example
        if(this->_automaton->_satExample == nullptr) {
            return Decision::UNSATISFIABLE;
        // Formula is valid if there is satisfying example and no unsatisfying example
        } else if(this->_automaton->_unsatExample == nullptr) {
            return Decision::VALID;
        // Formula is satisfiable if there exists satisfying example
        } else {
            return Decision::SATISFIABLE;
        }
    }
}

/**
 * Core of the Symbolic Decision procedure, Runs the procedure and prints the result according to the call of the
 * core function. Moreover prints various timings.
 */
void SymbolicChecker::Decide() {
    assert(this->_automaton != nullptr);

    try {
        Timer timer_deciding;
        bool decided;
        // Deciding WS1S formula
        timer_deciding.start();
        decided = this->Run();
        timer_deciding.stop();

        // Outing the results of decision procedure
        std::cout << "[!] Formula is ";
        switch(this->_DecideCore(decided)) {
            case Decision::SATISFIABLE:
                std::cout << "\033[1;34m'SATISFIABLE'\033[0m";
                break;
            case Decision::UNSATISFIABLE:
                std::cout << "\033[1;31m'UNSATISFIABLE'\033[0m";
                break;
            case Decision::VALID:
                std::cout << "\033[1;32m'VALID'\033[0m";
                break;
            case Decision::UNKNOWN:
                std::cout << "\033[1;33m'UNKNOWN'\033[0m";
                break;
            case Decision::INVALID:
                std::cout << "\033[1;36m'INVALID'\033[0m";
                break;
            default:
                std::cout << "undecided due to an error.\n";
                break;
        }

        std::cout << "\n";
        std::cout << "[*] Formula parse:      ";
        timer_parse.print();
        std::cout << "[*] Formula closure:    ";
        timer_closure.print();
        std::cout << "[*] Preprocessing:      ";
        timer_preprocess.print();
        std::cout << "[*] DFA creation:       ";
        timer_mona.print();
        std::cout << "[*] SA creation:        ";
        timer_automaton.print();
        std::cout << "[*] Decision procedure: ";
        timer_deciding.print();
        // Something that was used is not supported by dWiNA

#   if (DEBUG_GENERATE_DOT_AUTOMATON == true)
        if(options.printProgress) {
            SymbolicAutomaton::AutomatonToDot("automaton.dot", this->_automaton, false);
        }
#   endif

        Timer timer_clean_up;
        timer_clean_up.start();
        delete this->_automaton;
        this->_automaton = nullptr;
        timer_clean_up.stop();
        std::cout << "[*] Cleaning:           ";
        timer_clean_up.print();
    } catch (NotImplementedException& e) {
        std::cerr << e.what() << std::endl;
    }
}

int count_example_len(Term* t) {
    int len = 0;
    std::vector<Term_ptr> processed;
    while(t != nullptr && std::find_if(processed.begin(), processed.end(), [&t](Term_ptr i) { return t == i; }) == processed.end()) {
        if(t->link->val != "" && t->link->var == 0 || t->link->symbol != nullptr)
            ++len;
        processed.push_back(t);
        t = t->link->succ;
    }
    return len - 1;
    // Fixme:    ^-- i wonder why we need to sub -1 o.O
}

void sig_handler(int signum) {
    // Programming like a boss 8)
    throw GastonSignalException(signum);
}

void g_new_handler() {
    AST::temporalMapping.clear();
    SymbolicAutomaton::dagNegNodeCache->clear();
    SymbolicAutomaton::dagNodeCache->clear();
    throw GastonOutOfMemory();
}

/**
 * @brief Generates proof formula and prints to output the inferred examples
 *
 * @param[in]  example  example we are processing
 * @param[in]  exType  type of the example
 */
void SymbolicChecker::_ProcessExample(Term_ptr example, ExampleType exType) {
    if(example != nullptr) {
        VerificationResult proofVerification;
        std::ostringstream sout;

        sout.str(std::string()); // Clear the stream
        this->_automaton->DumpExample(sout, exType,
            (exType == ExampleType::SATISFYING ? this->_satInterpretation : this->_unsatInterpretation));

        if(DEBUG_GENERATE_PROOF_FORMULAE == true || options.verifyModels == true) {
            this->GenerateProofFormulaeFor(example, exType);
        }
        if(options.verifyModels) {
            proofVerification = this->VerifyProofFormula(this->GetProofFormulaFilename(exType));
        }
        if(exType == ExampleType::SATISFYING) {
            std::cout << "[*] Printing \033[1;32msatisfying\033[0m example of (";
        } else {
            assert(exType == ExampleType::UNSATISFYING);
            std::cout << "[*] Printing \033[1;31munsatisfying\033[0m example of (";
        }
        std::cout << (count_example_len(example)) << ") length";
        if(options.verifyModels) {
            std::cout << " [\033[" << VerificationResultToColour(proofVerification) << VerificationResultToString(proofVerification) << "\033[0m]";
        }
        std::cout << "\n";
        std::cout << sout.str();
        std::cout << "\n";
    }
}

/**
 * Runs the decision procedure on the constructed automaton
 */
bool SymbolicChecker::Run() {
    assert(this->_automaton != nullptr);
    std::cout << "\n[*] Deciding WS1S Symbolically\n";

    // Construct the initial approximation for final states
    // Note: This only copies the structure of fixpoint term with final
    //      states of base automata on leaves

    Term_ptr finalStatesApproximation = this->_automaton->GetFinalStates();
#   if (DEBUG_INITIAL_APPROX == true)
    finalStatesApproximation->dump();
    std::cout << "\n";
#   endif

    // Initialize signal handlers for timeouts, in order to be polite and clean
    signal(SIGINT, sig_handler);
#   if (DEBUG_DONT_CATCH_SIGSEGV == false)
    signal(SIGSEGV, sig_handler); // This might be suicidal though
#   endif
    std::set_new_handler(g_new_handler);

    // Checks if Initial States intersect Final states
    std::pair<Term_ptr, bool> result;
    try {
        result = this->_automaton->IntersectNonEmpty(nullptr, finalStatesApproximation, IntersectNonEmptyParams(false));
    } catch (const GastonSignalException& exception) {
        std::cout << exception.what() << "\n";
        this->_terminatedBySignal = true;
    } catch (const GastonOutOfMemory& exception) {
        std::cout << exception.what() << "\n";
        this->_terminatedBySignal = true;
    }
    Term_ptr fixpoint = result.first;
    bool isValid = result.second;

#   if (DUMP_EXAMPLES == true)
    this->_ProcessExample(this->_automaton->_satExample, ExampleType::SATISFYING);
    this->_ProcessExample(this->_automaton->_unsatExample, ExampleType::UNSATISFYING);
#   endif

#   if (DEBUG_FIXPOINT == true)
    std::cout << "[!] Finished deciding WS1S formula with following fixpoint:\n";
    if(fixpoint != nullptr) {
        fixpoint->dump();
#       if (DEBUG_GENERATE_DOT_FIXPOINT == true)
        std::ofstream os;
        os.open(std::string("fixpoint.dot"));
        Term::ToDot(fixpoint, os);
#       endif
    }
    std::cout << "\n";
#   endif

    std::cout << "\t\u2218 Partial Subsumption Hits: " << Term::partial_subsumption_hits << "\n";
#   if (MEASURE_STATE_SPACE == true)
#   define OUTPUT_MEASURES(TermType) \
        std::cout << "\t\t\u2218 prunnable: " << TermType::prunable << "\n"; \
        std::cout << "\t\t\u2218 (==) by same ptr: " << TermType::comparisonsBySamePtr << "/" << Term::comparisonsBySamePtr; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsBySamePtr/(double)Term::comparisonsBySamePtr)*100 <<"%)\n"; \
        std::cout << "\t\t\u2218 (==) by diff type: " << TermType::comparisonsByDiffType << "/" << Term::comparisonsByDiffType; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsByDiffType/(double)Term::comparisonsByDiffType)*100 <<"%)\n"; \
        std::cout << "\t\t\u2218 (==) by structure: " << TermType::comparisonsByStructureTrue << " trues /" << TermType::comparisonsByStructureFalse << " falses\n";

    std::cout << "[*] Measured State Space: \n";
    std::cout << "\t\u2218 Symbols: " << ZeroSymbol::instances << "\n";
    std::cout << "\t\u2218 Term Empty: " << TermEmpty::instances << "\n";
    std::cout << "\t\u2218 Term Products: " << TermProduct::instances << "\n";
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermProduct)
#   endif
#   if (OPT_USE_TERNARY_AUTOMATA == true)
    std::cout << "\t\u2218 Term TernaryProducts: " << TermTernaryProduct::instances << "\n";
#   endif
#   if (OPT_USE_NARY_AUTOMATA == true)
    std::cout << "\t\u2218 Term NaryProducts: " << TermNaryProduct::instances << "\n";
#   endif
    std::cout << "\t\u2218 Term Bases: " << TermBaseSet::instances;
#   if (MEASURE_BASE_SIZE == true)
    std::cout << " (max size = " << TermBaseSet::maxBaseSize << ")";
#   endif
    std::cout << "\n";
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermBaseSet)
#   endif
    std::cout << "\t\u2218 Term Fixpoints: " << TermFixpoint::instances;
    std::cout << " (" << (TermFixpoint::instances - TermFixpoint::preInstances) << " + " << TermFixpoint::preInstances <<")";
    std::cout << " fully computed: " << std::setprecision(2) << (TermFixpoint::fullyComputedFixpoints / (double) TermFixpoint::instances)*100 << "%";
    std::cout << "\n";
#   if (MEASURE_PROJECTION == true)
    std::cout << "\t\t\u2218 is not shared: " << TermFixpoint::isNotShared << "\n";
#   endif
#   if (MEASURE_SUBSUMEDBY_HITS == true)
    std::cout << "\t\t\u2218 subsumedBy hits: " << TermFixpoint::subsumedByHits << "\n";
#   endif
#   if (MEASURE_POSTPONED == true)
    std::cout << "\t\t\u2218 postponed terms: " << TermFixpoint::postponedTerms << " (" << TermFixpoint::postponedProcessed << " evaluated)\n";
#   endif
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermFixpoint)
#   endif
    std::cout << "\t\u2218 Term Lists: " << TermList::instances << "\n";
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermList)
#   endif
    std::cout << "\t\u2218 Term Continuations: " << (TermContinuation::instances) << "\n";
#   if (MEASURE_CONTINUATION_EVALUATION == true)
    std::cout << "\t\t\u2218 evaluated: " << TermContinuation::continuationUnfolding << "\n";
    std::cout << "\t\t\t\u2218 in subsumption: " << TermContinuation::unfoldInSubsumption << "\n";
    std::cout << "\t\t\t\u2218 in isect nonempty: " << TermContinuation::unfoldInIsectNonempty << "\n";
#   endif
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermContinuation)
#   endif
#   undef OUTPUT_MEASURES
    std::cout << "[*] Mona State Space: " << (Workshops::TermWorkshop::monaAutomataStates) << "\n";
    std::cout << "[*] Overall State Space: " << (TermProduct::instances + TermBaseSet::instances + TermFixpoint::instances
                                                 + TermList::instances + TermContinuation::instances + TermNaryProduct::instances
                                                 + TermTernaryProduct::instances + Workshops::TermWorkshop::monaAutomataStates) << "\n";
    std::cout << "[*] Explored Fixpoint Space: " << (fixpoint != nullptr ? fixpoint->MeasureStateSpace() : 0) << "\n";
#   endif

#   if (PRINT_STATS == true)
    if (options.printProgress) {
        std::cout << "[*] Printing Statistics\n";
        this->_automaton->DumpComputationStats();
        std::cout << "\n";
    }
#   endif

    // If Initial States does intersect final ones, the formula is valid, else it is unsatisfiable
    return isValid;
}

/**
 * @brief Generates name for the proof formula
 *
 * Generates filename for the proof for satisfying/unsatisfying example in form:
 *   'inputFilename_(un)satisfying.mona'
 *
 * @param[in]  exType  type of the example, either satisfying or unsatisfying
 * @return  filename for the formula
 */
std::string SymbolicChecker::GetProofFormulaFilename(ExampleType exType) {
    std::string path(inputFileName);
    path  = path.substr(0, path.find_last_of("."));
    path += (exType == ExampleType::SATISFYING ? "_satisfying" : "_unsatisfying");
    path += ".mona";
    return path;
}

/**
 * @brief Generates formula that serves as proof for @p example
 *
 * Generates formula that will server as a proof of validity of satisfying @p example of @p exType.
 * Ground formula is constructed that restricts the model. The validity of constructed formula
 * implies validity of inferred (un)satisfying example.
 *
 * @param[in]  example  term that represents the inferred example
 * @param[in]  exType  type of the example
 */
void SymbolicChecker::GenerateProofFormulaeFor(Term_ptr example, ExampleType exType) {
    // Construct the name
    std::string path = this->GetProofFormulaFilename(exType);

    // Construct the formulae
    ASTForm* toSerialize;
    Derestricter derestricter;
    toSerialize = static_cast<ASTForm*>(this->_beforeClosure->accept(derestricter));

    std::string first_orders;
    std::string second_orders;
    std::string model;
    auto it = (exType == ExampleType::SATISFYING ? this->_satInterpretation.begin() : this->_unsatInterpretation.begin());
    auto end = (exType == ExampleType::SATISFYING ? this->_satInterpretation.end() : this->_unsatInterpretation.end());
    for(; it != end; ++it) {
        if(it->find("}") == std::string::npos) {
            if(first_orders.size() != 0) {
                first_orders += ", " + it->substr(0, it->find_last_of("="));
            } else {
                first_orders += it->substr(0, it->find_last_of("="));
            }
        } else {
            // Second orders
            if(second_orders.size() != 0) {
                second_orders += ", " + it->substr(0, it->find_last_of("="));
            } else {
                second_orders += it->substr(0, it->find_last_of("="));
            }
        }

        model += *it + " & ";
    }

    std::ofstream proofFile;
    proofFile.open(path);
    if(!proofFile.is_open()) {
        throw std::ios_base::failure("Unable to open proof file");
    }

    if(allPosVar == -1) {
        proofFile << "ws1s;\n";
    } else {
        proofFile << "m2l-str;\n";
    }

    assert(first_orders.size() > 0 || second_orders.size() > 0);

    if(first_orders.size() != 0) {
        proofFile << "ex1 " << first_orders << ": ";
    }
    if(second_orders.size() != 0) {
        proofFile << "ex2 " << second_orders << ": ";
    }
    proofFile << model;
    proofFile << (exType == ExampleType::SATISFYING ? "" : "~");
    proofFile << "(" << toSerialize->ToString(true) << ");\n";
    proofFile.close();
}

/**
 * @brief Verifies generated proof formula by running the mona and checking the validity of formulae
 *
 * Takes the generated proof formula @p filename, opens process and runs MONA on the generated
 * formula to test the validity of this formula. If the proof formula is valid, then we can say
 * that the model is correct.
 *
 * @param[in]  filename  filename with the proof formula to be verified
 * @return  UNKNOWN if error occured during verification process or opening of pipe,
 *          VERIFIED if proof formula is valid
 *          INCORRECT if proof formula is unsatisfiable
 */
VerificationResult SymbolicChecker::VerifyProofFormula(std::string filename) {
    FILE *fpipe;
    // Construct command for running mona
    std::string command("mona");
    command += " -q ";
    command += filename;
    char line[256];

    // Try to open and run mona
    if(!(fpipe = (FILE*) popen(command.c_str(), "r"))) {
        return VerificationResult::UNKNOWN;
    }

    VerificationResult res = VerificationResult::UNKNOWN;
    while(fgets(line, sizeof(line), fpipe)) {
        if(!strcmp(line, "Formula is valid\n")) {
            res = VerificationResult::VERIFIED;
            break;
        } else if(!strcmp(line, "Formula is unsatisfiable\n")) {
            res = VerificationResult::INCORRECT;
            break;
        }
    }

    pclose(fpipe);
    return res;
}