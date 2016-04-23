//
// Created by Raph on 27/01/2016.
//

#include "Tagger.h"

void Tagger::_tagFormula(ASTForm *form) {
	if(_tit != this->_tagList.end() && this->_lastTag == (*_tit)) {
		form->tag = 0;
		_tit = this->_tagList.erase(_tit);
		++this->_lastTag;
	} else {
		form->tag = this->_lastTag++;
	};
}

void Tagger::visit(ASTForm_And *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_Or *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_Impl *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_Biimpl *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_Not *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_Ex1 *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_Ex2 *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_All1 *form) {
	this->_tagFormula(form);
}

void Tagger::visit(ASTForm_All2 *form) {
	this->_tagFormula(form);
}