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
extern void (*mona_callback)();
extern Deque<FileSource *> source;

// Fixme: remove maybe?
extern char *inputFileName;

extern Ident lastPosVar, allPosVar;

Checker::~Checker() {
    // Clean up
    delete _monaAST;
    Deque<FileSource *>::iterator i;
    for (i = source.begin(); i != source.end(); i++)
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
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tags.insert(tags.end(), std::stoi(token));
        s.erase(0, pos + delimiter.length());
    }
    tags.insert(tags.end(), std::stoi(s));
}

void Checker::LoadFormulaFromFile() {
    loadFile(inputFileName);
    yyparse();
    this->_monaAST = untypedAST->typeCheck();
    lastPosVar = this->_monaAST->lastPosVar;
    allPosVar = this->_monaAST->allPosVar;

    // Prints progress if dumping is set
    if (options.printProgress) {
        G_DEBUG_FORMULA_AFTER_PHASE("loading");
        std::cout << "[*] Elapsed time: ";
    }

    // Clean up untypedAST
    delete untypedAST;
}

void Checker::CloseUngroundFormula() {
    assert(this->_monaAST != nullptr);

    // First close the formula if we are testing something specific
    IdentList freeVars, bound;
    (this->_monaAST->formula)->freeVars(&freeVars, &bound);
    bool formulaIsGround = freeVars.empty();
    if(!formulaIsGround) {
        switch(options.test) {
            case TestType::VALIDITY:
                // Fixme: this is incorrect, we need to tell that the variable is in first order
                this->_monaAST->formula = new ASTForm_All1(nullptr, &freeVars, this->_monaAST->formula, Pos());
                break;
            case TestType::SATISFIABILITY:
                this->_monaAST->formula = new ASTForm_Ex1(nullptr, &freeVars, this->_monaAST->formula, Pos());
                break;
            case TestType::UNSATISFIABILITY:
                this->_monaAST->formula = new ASTForm_Ex1(nullptr, &freeVars, new ASTForm_Not(this->_monaAST->formula, Pos()), Pos());
                break;
            default:
                assert(false && "Cannot handle the unground formulae right now.");
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