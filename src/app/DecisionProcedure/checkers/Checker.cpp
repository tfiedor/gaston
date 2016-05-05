//
// Created by Raph on 02/02/2016.
//

#include "Checker.h"
#include "../../Frontend/predlib.h"
#include "../../Frontend/untyped.h"
#include "../../Frontend/symboltable.h"
#include "../containers/VarToTrackMap.hh"
#include "../environment.hh"

#include "../visitors/transformers/AntiPrenexer.h"
#include "../visitors/transformers/BooleanUnfolder.h"
#include "../visitors/transformers/PrenexNormalFormTransformer.h"
#include "../visitors/transformers/Reorderer.h"
#include "../visitors/transformers/BinaryReorderer.h"
#include "../visitors/transformers/QuantificationMerger.h"
#include "../visitors/transformers/BaseAutomataMerger.h"
#include "../visitors/transformers/ExistentialPrenexer.h"
#include "../visitors/transformers/ShuffleVisitor.h"
#include "../visitors/transformers/ContinuationSwitcher.h"
#include "../visitors/restricters/UniversalQuantifierRemover.h"
#include "../visitors/restricters/SyntaxRestricter.h"
#include "../visitors/restricters/SecondOrderRestricter.h"
#include "../visitors/restricters/Flattener.h"
#include "../visitors/restricters/PredicateUnfolder.h"
#include "../visitors/restricters/NegationUnfolder.h"
#include "../visitors/restricters/ZeroOrderRemover.h"
#include "../visitors/printers/DotWalker.h"
#include "../visitors/printers/MonaAutomataDotWalker.h"
#include "../visitors/printers/MonaSerializer.h"
#include "../visitors/decorators/UnderComplementDecorator.h"
#include "../visitors/decorators/Tagger.h"
#include "../visitors/decorators/FixpointDetagger.h"
#include "../visitors/decorators/OccuringVariableDecorator.h"

extern PredicateLib predicateLib;

extern Options options;
extern MonaUntypedAST *untypedAST;
extern SymbolTable symbolTable;
extern VarToTrackMap varMap;
extern CodeTable *codeTable;

extern int yyparse(void);
extern void loadFile(char *filename);
//extern void (*mona_callback)();
extern Deque<FileSource *> source;

// Fixme: remove maybe?
extern char *inputFileName;
extern Ident lastPosVar, allPosVar;
extern Timer timer_preprocess;

Checker::~Checker() {
    // Clean up
    delete _monaAST;
    Deque<FileSource *>::iterator i;
    for (i = source.begin(); i != source.end(); ++i)
        delete *i;

    PredLibEntry *pred = predicateLib.first();
    while(pred != nullptr) {
        delete pred->ast;
        pred = predicateLib.next();
    }
}

/**
 * Initialize the Map of variables between MONA and the decision procedure
 *
 * @param[in] formula:  input formulae
 */
void initializeVarMap(ASTForm* formula) {
    assert(formula != nullptr);

    IdentList free, bound;
    formula->freeVars(&free, &bound);
#   if (DEBUG_VARMAP == true)
    std::cout << "Free Variables: "; free.dump(); std::cout << "\n";
    std::cout << "Bound Variables: "; bound.dump(); std::cout << "\n";
#   endif

    IdentList *vars = ident_union(&free, &bound);
    if (vars != nullptr) {
        varMap.initializeFromList(vars);
    }
    delete vars;
}

/**
 * Reads the Tags from the @p filename and outputs them into the stream @p output. Tags are further used for some
 * optimizations, mostly for transforming subformulae to the automaton
 *
 * @param[in] filename: input filename we are reading from
 * @param[out] output:  string stream we are outing to
 */
void readTags(char* filename, std::string& output) {
    std::ifstream infile(filename);
    std::getline(infile, output);
    infile.close();
}

/**
 * Parse the input strings to the tags.
 *
 * @param[in] s:     input string with tags
 * @param[out] tags: list of parsed tags
 */
void parseTags(std::string&s, std::list<size_t>& tags) {
    std::string delimiter = ";";
    std::string beginDelimiter = ":";
    size_t pos = 0;

    if((pos = s.find(beginDelimiter)) == std::string::npos) {
        return;
    }
    s = s.substr(pos+1);
    std::string token;
    try {
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tags.insert(tags.end(), std::stoi(token));
            s.erase(0, pos + delimiter.length());
        }
        tags.insert(tags.end(), std::stoi(s));
    } catch (const std::invalid_argument& e) {
        // We'll skip the rest of the tags that were malformed
        std::cout << "[!] \033[1;31mWarning\033[0m: Invalid tag rest were skipped\n";
    }
}

/**
 * Starts the timer @p t
 *
 * @param[in] t: timer we are starting
 */
void Checker::_startTimer(Timer &t) {
    t.start();
}

/**
 * Stops the timer and outputs the measured time
 *
 * @param[in] t:                    timer we are stopping
 * @param[in] additionalMessage:    additional stuff we are outing
 */
void Checker::_stopTimer(Timer &t, char* additionalMessage) {
    t.stop();
    if(this->_printProgress) {
        std::cout << "[*] " << additionalMessage;
        std::cout << "Elapsed time: ";
        t.print();
        std::cout << "\n";
    }
}

/**
 * Reads the formulae from file and parses it into the AST representation
 */
void Checker::LoadFormulaFromFile() {
    loadFile(inputFileName);
    yyparse();
    this->_monaAST = untypedAST->typeCheck();
    lastPosVar = this->_monaAST->lastPosVar;
    allPosVar = this->_monaAST->allPosVar;
    // Clean up untypedAST
    delete untypedAST;
}

/**
 * Closes the prefix of the unground @p formula according tot he list of @p freeVars
 *
 * @param[in] freeVars:     list of free variables
 * @param[in] formula:      formula we are closing
 */
template<class ZeroOrderQuantifier, class FirstOrderQuantifier, class SecondOrderQuantifier>
ASTForm* Checker::_ClosePrefix(IdentList* freeVars, ASTForm* formula) {
    IdentList zeroOrders, firstOrders, secondOrders;
    for(auto it = freeVars->begin(); it != freeVars->end(); ++it) {
        // AllPos var is skipped
        if(*it == allPosVar)
            continue;
        // Distribute the first order and second order variables
        switch(symbolTable.lookupType(*it)) {
            case MonaTypeTag::Varname0:
                zeroOrders.push_back(*it);
                break;
            case MonaTypeTag::Varname1:
                firstOrders.push_back(*it);
                break;
            case MonaTypeTag::Varname2:
                secondOrders.push_back(*it);
                break;
            default:
                assert(false && "Impossible happened!");
        }
    }

    if(!zeroOrders.empty()) {
        formula = new ZeroOrderQuantifier(zeroOrders.copy(), formula, Pos());
    }
    if(!secondOrders.empty()) {
        formula = new SecondOrderQuantifier(nullptr, secondOrders.copy(), formula, Pos());
    }
    if(!firstOrders.empty()) {
        formula = new FirstOrderQuantifier(nullptr, firstOrders.copy(), formula, Pos());
    }

    this->_isGround = true;
    return formula;
};

void Checker::CloseUngroundFormula() {
    assert(this->_monaAST != nullptr);

    // First close the formula if we are testing something specific
    IdentList freeVars, bound;
    (this->_monaAST->formula)->freeVars(&freeVars, &bound);

    //this->_isGround = freeVars.empty() || (freeVars.size() == 1 && *freeVars.begin() == allPosVar);
    this->_isGround = freeVars.empty();
    if(!this->_isGround && !(freeVars.size() == 1 && *freeVars.begin() == allPosVar)) {
        switch(options.test) {
            case TestType::VALIDITY:
                this->_monaAST->formula = this->_ClosePrefix<ASTForm_All0, ASTForm_All1, ASTForm_All2>(&freeVars, this->_monaAST->formula);
                break;
            case TestType::SATISFIABILITY:
                this->_monaAST->formula = this->_ClosePrefix<ASTForm_Ex0, ASTForm_Ex1, ASTForm_Ex2>(&freeVars, this->_monaAST->formula);
                break;
            case TestType::UNSATISFIABILITY:
                this->_monaAST->formula = this->_ClosePrefix<ASTForm_Ex0, ASTForm_Ex1, ASTForm_Ex2>(&freeVars, new ASTForm_Not(this->_monaAST->formula, Pos()));
                break;
            default:
                // We will test everything
                ASTForm* restrictions = new ASTForm_True(Pos());
                for(auto it = freeVars.begin(); it != freeVars.end(); ++it) {
                    // Fixme: what about zeroorder variables
                    if(symbolTable.lookupType(*it) == MonaTypeTag::Varname1) {
                        restrictions
                                = SecondOrderRestricter::RestrictFormula<ASTForm_And, ASTTerm1_Var1>(*it, restrictions);
                    } else if(symbolTable.lookupType(*it) == MonaTypeTag::Varname2 && *it != allPosVar) {
                        restrictions
                                = SecondOrderRestricter::RestrictFormula<ASTForm_And, ASTTerm2_Var2>(*it, restrictions);
                    }
                }
                BooleanUnfolder booleanUnfolder;
                this->_rootRestriction = static_cast<ASTForm*>(restrictions->accept(booleanUnfolder));
                FixpointDetagger detagger;
                this->_rootRestriction->accept(detagger);
                break;
        }
    }
}

/**
 * Calls the preprocessing filters according to the list defined in Checker.h. Then calls some additional phases
 * that are common for all of the options of prenexing: tags the nodes by unique ids, then run the detagger to remove
 * some of the tags in order to transform them into the subautomata. Finally everything is restricted to second order.
 */
void Checker::PreprocessFormula() {
    this->_startTimer(timer_preprocess);
    // Flattening of the formula
    PredicateUnfolder predicateUnfolder;
    this->_monaAST->formula = static_cast<ASTForm *>((this->_monaAST->formula)->accept(predicateUnfolder));

    if (options.dump) {
        G_DEBUG_FORMULA_AFTER_PHASE("Predicate Unfolding");
        (this->_monaAST->formula)->dump();
    }

        // Additional filter phases
#define CALL_FILTER(filter) \
        if(!strcmp(#filter, "FullAntiPrenexer")) { \
            OccuringVariableDecorator decorator; \
            this->_monaAST->formula->accept(decorator); \
        } \
        filter filter##_visitor;    \
        this->_monaAST->formula = static_cast<ASTForm *>(this->_monaAST->formula->accept(filter##_visitor));    \
        if(options.dump) {    \
            G_DEBUG_FORMULA_AFTER_PHASE(#filter);    \
            (this->_monaAST->formula)->dump();    std::cout << "\n";    \
        } \
        if(options.graphvizDAG) {\
            std::string filter##_dotName(""); \
            filter##_dotName += #filter; \
            filter##_dotName += ".dot"; \
            DotWalker filter##_dw_visitor(filter##_dotName); \
            (this->_monaAST->formula)->accept(filter##_dw_visitor); \
        }
    FILTER_LIST(CALL_FILTER)
#undef CALL_FILTER

    if (options.serializeMona) {
        std::string path(inputFileName);
        MonaSerializer serializer(path.substr(0, path.find_last_of(".")) + "_serialized.mona", this->_monaAST->formula);
    }

    std::list <size_t> tags;
    std::string stringTag("");
    readTags(inputFileName, stringTag);
    parseTags(stringTag, tags);

#   if (OPT_SHUFFLE_FORMULA == true)
    ShuffleVisitor shuffleVisitor;
    this->_monaAST->formula = static_cast<ASTForm *>(this->_monaAST->formula->accept(shuffleVisitor));
#   endif

    Tagger tagger(tags);
    (this->_monaAST->formula)->accept(tagger);

    FixpointDetagger detagger;
    (this->_monaAST->formula)->accept(detagger);

    if (!options.monaWalk) {
        SecondOrderRestricter restricter;
        this->_monaAST->formula = static_cast<ASTForm *>((this->_monaAST->formula)->accept(restricter));

        QuantificationMerger quantificationMerger;
        this->_monaAST->formula = static_cast<ASTForm *>((this->_monaAST->formula)->accept(quantificationMerger));

#       if (OPT_EARLY_EVALUATION == true)
        UnderComplementDecorator underComplementDecorator;
        (this->_monaAST->formula)->accept(underComplementDecorator);

        ContinuationSwitcher continuationSwitcher;
        (this->_monaAST->formula)->accept(continuationSwitcher);
#       endif
    }

    if(options.graphvizDAG) {
        std::string dotFileName(inputFileName);
        dotFileName += ".dot";
        DotWalker dw_visitor(dotFileName);
        (this->_monaAST->formula)->accept(dw_visitor);
    }

    this->_stopTimer(timer_preprocess, "Preprocess");

    // Table or BDD tracks are reordered
    initializeVarMap(this->_monaAST->formula);
    AST::temporalMapping.resize(symbolTable.noIdents);
    std::fill(AST::temporalMapping.begin(), AST::temporalMapping.end(), 0);
}

/**
 * Special experiment, that tries to conver every possible subformula into corresponding automaton.
 */
void Checker::CreateAutomataSizeEstimations() {
    std::string monaDot(inputFileName);
    monaDot += "-walk.dot";
    MonaAutomataDotWalker monaWalker(monaDot, true);
    (this->_monaAST->formula)->accept(monaWalker);
}