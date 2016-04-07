//
// Created by Raph on 02/02/2016.
//

#include "Checker.h"
#include "../../Frontend/predlib.h"
#include "../../Frontend/untyped.h"
#include "../../Frontend/symboltable.h"
#include "../containers/VarToTrackMap.hh"
#include "../environment.hh"

#include "../visitors/AntiPrenexer.h"
#include "../visitors/BooleanUnfolder.h"
#include "../visitors/UniversalQuantifierRemover.h"
#include "../visitors/SyntaxRestricter.h"
#include "../visitors/SecondOrderRestricter.h"
#include "../visitors/PrenexNormalFormTransformer.h"
#include "../visitors/Flattener.h"
#include "../visitors/PredicateUnfolder.h"
#include "../visitors/NegationUnfolder.h"
#include "../visitors/Reorderer.h"
#include "../visitors/BinaryReorderer.h"
#include "../visitors/QuantificationMerger.h"
#include "../visitors/DotWalker.h"
#include "../visitors/MonaAutomataDotWalker.h"
#include "../visitors/BaseAutomataMerger.h"
#include "../visitors/ExistentialPrenexer.h"
#include "../visitors/ZeroOrderRemover.h"
#include "../visitors/Tagger.h"
#include "../visitors/FixpointDetagger.h"

extern PredicateLib predicateLib;

extern Options options;
extern MonaUntypedAST *untypedAST;
extern SymbolTable symbolTable;
extern VarToTrackMap varMap;

extern int yyparse(void);
extern void loadFile(char *filename);
//extern void (*mona_callback)();
extern Deque<FileSource *> source;

// Fixme: remove maybe?
extern char *inputFileName;

extern Ident lastPosVar, allPosVar;

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
 * Implementation is not sure at the moment, but should reorder the symbol
 * table or BDD track, so it is optimized for using of projection during
 * the decision procedure process. It should consider the structure of prefix
 * of given formula, so BDD used in transitions of automata can be better
 * reordered
 */
void initializeVarMap(ASTForm* formula) {
    IdentList free, bound;
    formula->freeVars(&free, &bound);
#   if (DEBUG_VARMAP == true)
    std::cout << "Free Variables: "; free.dump(); std::cout << "\n";
    std::cout << "Bound Variables: "; bound.dump(); std::cout << "\n";
#   endif

    IdentList *vars = ident_union(&free, &bound);
    if (vars != 0) {
        varMap.initializeFromList(vars);
    }
    delete vars;
}

void readTags(char* filename, std::string& output) {
    std::ifstream infile(filename);
    std::getline(infile, output);
    infile.close();
}

void parseTags(std::string&s, std::list<size_t>& tags) {
    std::string delimiter = ";";
    std::string beginDelimiter = ":";
    // Fixme: better things

    size_t pos = 0;

    pos = s.find(beginDelimiter);
    if(pos == std::string::npos) {

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
    } catch (std::invalid_argument e) {
        std::cout << "[!] \033[1;31mWarning\033[0m: Invalid tag rest were skipped\n";
    }
}

void Checker::_startTimer(Timer &t) {
    t.start();
}

void Checker::_stopTimer(Timer &t, char* additionalMessage) {
    t.stop();
    if(this->_printProgress) {
        std::cout << "[*] " << additionalMessage;
        std::cout << "Elapsed time: ";
        t.print();
        std::cout << "\n";
    }
}

void Checker::LoadFormulaFromFile() {
    loadFile(inputFileName);
    yyparse();
    this->_monaAST = untypedAST->typeCheck();
    lastPosVar = this->_monaAST->lastPosVar;
    allPosVar = this->_monaAST->allPosVar;
    // Clean up untypedAST
    delete untypedAST;
}

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
                for(auto it = freeVars.begin(); it != freeVars.end(); ++it) {
                    // Fixme: what about zeroorder variables
                    if(symbolTable.lookupType(*it) == MonaTypeTag::Varname1) {
                        ASTForm_FirstOrder* fo = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, Pos()), Pos());
                        this->_monaAST->formula = new ASTForm_And(fo, this->_monaAST->formula, Pos());
                        this->_monaAST->formula
                                = SecondOrderRestricter::RestrictFormula<ASTForm_And, ASTTerm1_Var1>(*it, this->_monaAST->formula);
                    } else if(symbolTable.lookupType(*it) == MonaTypeTag::Varname2 && *it != allPosVar) {
                        this->_monaAST->formula
                                = SecondOrderRestricter::RestrictFormula<ASTForm_And, ASTTerm2_Var2>(*it, this->_monaAST->formula);
                    }
                }
                break;
        }
    }
}

void Checker::PreprocessFormula() {
    // Flattening of the formula
    PredicateUnfolder predicateUnfolder;
    this->_monaAST->formula = static_cast<ASTForm *>((this->_monaAST->formula)->accept(predicateUnfolder));

    if (options.dump) {
        G_DEBUG_FORMULA_AFTER_PHASE("Predicate Unfolding");
        (this->_monaAST->formula)->dump();
    }

    // Additional filter phases
    #define CALL_FILTER(filter) \
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

    std::list<size_t> tags;
    std::string stringTag("");
    readTags(inputFileName, stringTag);
    parseTags(stringTag, tags);

    Tagger tagger(tags);
    (this->_monaAST->formula)->accept(tagger);

    FixpointDetagger detagger;
    (this->_monaAST->formula)->accept(detagger);

    SecondOrderRestricter restricter;
    this->_monaAST->formula = static_cast<ASTForm*>((this->_monaAST->formula)->accept(restricter));

    if(options.graphvizDAG) {
        std::string dotFileName(inputFileName);
        dotFileName += ".dot";
        DotWalker dw_visitor(dotFileName);
        (this->_monaAST->formula)->accept(dw_visitor);
    }

    // Table or BDD tracks are reordered
    initializeVarMap(this->_monaAST->formula);
}

void Checker::CreateAutomataSizeEstimations() {
    std::string monaDot(inputFileName);
    monaDot += "-walk.dot";
    MonaAutomataDotWalker monaWalker(monaDot);
    (this->_monaAST->formula)->accept(monaWalker);
}