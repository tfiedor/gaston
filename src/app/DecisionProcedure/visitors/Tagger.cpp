//
// Created by Raph on 27/01/2016.
//

#include "Tagger.h"

void Tagger::visit(ASTForm_And *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_Or *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_Impl *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_Biimpl *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_Not *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_Ex1 *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_Ex2 *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_All1 *form) {
	form->tag = this->_lastTag++;
}

void Tagger::visit(ASTForm_All2 *form) {
	form->tag = this->_lastTag++;
}