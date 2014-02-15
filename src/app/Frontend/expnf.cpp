/*
 * MONA
 * Copyright (C) 1997-2013 Aarhus University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the  Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335,
 * USA.
 */

#include "ast.h"
#include "symboltable.h"

#include <cstring>

using std::cout;

extern SymbolTable symbolTable;

void 
ASTComponent::cdump()
{
  cout << name;
  if (path)
    path->dump();
  else
    cout << "[?]";
  cout << ":" << type;
}

void
ASTComponentList::cdump()
{
  for (iterator i = begin(); i != end(); i++) {
    if (i != begin())
      cout << ",";
    (*i)->cdump();
  }
}

void
ASTVariant::cdump()
{
  cout << name;
  if (path)
    path->cdump();
  else
    cout << "[?]";
  cout << "(";
  if (components)
    components->cdump();
  cout << ")";
}

void
ASTVariantList::cdump()
{
  for (iterator i = begin(); i != end(); i++) {
    if (i != begin())
      cout << ",";
    (*i)->cdump();
  }
}

void
BitList::cdump()
{
  cout << "[";
  for (iterator i = begin(); i != end(); i++)
    if (*i == Zero)
      cout << "0";
    else
      cout << "1";
  cout << "]";
}

void 
ASTList::cdump()
{
  iterator i;
  for (i = begin(); i != end(); i++) {
    cout << ","; (*i)->cdump();
  }
}

void 
ASTUniv::cdump()
{
  cout << symbolTable.lookupSymbol(u);
}

void 
ASTTerm1_Var1::cdump()
{
  cout << "Var1 " << symbolTable.lookupSymbol(n);
}
  
void 
ASTTerm1_Dot::cdump()
{
  cout << "Dot("; t->dump(); cout << ",";
  BitList::iterator i;
  for (i = bits->begin(); i != bits->end(); i++)
    if (*i == Zero)
      cout << "0";
    else
      cout << "1";
  cout << ")";
}
    
void 
ASTTerm1_Up::cdump()
{
  cout << "Up("; t->cdump(); cout << ")";
}
    
void
ASTTerm1_Root::cdump()
{
  if (univ == -1)
    cout << "Root";
  else
    cout << "Root(" << symbolTable.lookupSymbol(univ) << ")";
}

void 
ASTTerm1_Int::cdump()
{
  cout << "Int " << n;
}
    
void 
ASTTerm1_Plus::cdump()
{
  cout << "Plus1("; t->cdump(); cout << "," << n << ")";
}
    
void 
ASTTerm1_Minus::cdump()
{
  cout << "Minus1("; t->cdump(); cout << "," << n << ")";
}
    
void 
ASTTerm1_PlusModulo::cdump()
{
  cout << "PlusModulo1("; t1->cdump(); cout << ","  << n << ",";
  t2->cdump(); cout << ")";
}
    
void 
ASTTerm1_MinusModulo::cdump()
{
  cout << "MinusModulo1("; t1->cdump(); cout << "," << n << ",";
  t2->cdump(); cout << ")";
}
    
void 
ASTTerm1_Min::cdump()
{
  cout << "Min("; T->cdump(); cout << ")";
}
    
void 
ASTTerm1_Max::cdump()
{
  cout << "Max("; T->cdump(); cout << ")";
}
    
void 
ASTTerm1_TreeRoot::cdump()
{
  cout << "TreeRoot("; T->cdump(); cout << ")";
}
    
void 
ASTTerm2_Var2::cdump()
{
  cout << "Var2 " << symbolTable.lookupSymbol(n);
}
    
void 
ASTTerm2_VarTree::cdump()
{
  cout << "Tree " << symbolTable.lookupSymbol(n);
}
    
void 
ASTTerm2_Dot::cdump()
{
  cout << "Dot("; T->dump(); cout << ",";
  BitList::iterator i;
  for (i = bits->begin(); i != bits->end(); i++)
    if (*i == Zero)
      cout << "0";
    else
      cout << "1";
  cout << ")";
}
    
void 
ASTTerm2_Up::cdump()
{
  cout << "Up("; T->cdump(); cout << ")";
}
    
void 
ASTTerm2_Empty::cdump()
{
  cout << "Empty";
}
    
void 
ASTTerm2_Union::cdump()
{
  cout << "Union("; T1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTTerm2_Inter::cdump()
{
  cout << "Inter("; T1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTTerm2_Setminus::cdump()
{
  cout << "Setminus("; T1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTTerm2_Set::cdump()
{
  ASTList::iterator i;
  cout << "Set(";
  for (i = elements->begin(); i != elements->end();) {
    (*i)->cdump();
    if (++i != elements->end()) 
      cout << ",";
  }
  cout << ")";
}
    
void 
ASTTerm2_Plus::cdump()
{
  cout << "Plus2("; T->cdump(); cout << "," << n << ")";
}
    
void 
ASTTerm2_Minus::cdump()
{
  cout << "Minus2("; T->cdump(); cout << "," << n << ")";
}
    
void 
ASTTerm2_Interval::cdump()
{
  cout << "Interval("; t1->cdump(); cout << ","; t2->cdump(); cout << ")";
}
    
void 
ASTTerm2_PresbConst::cdump()
{
  cout << "PresbConst(" << value << ")"; 
}

void 
ASTTerm2_Formula::cdump()
{
  cout << "Term2Formula(" << symbolTable.lookupSymbol(fresh) << ",";
  f->cdump(); cout << ")";
}

void 
ASTForm_Var0::cdump()
{
  cout << "Var0 " << symbolTable.lookupSymbol(n);
}
    
void 
ASTForm_True::cdump()
{
  cout << "True";
}
    
void 
ASTForm_False::cdump()
{
  cout << "False";
}
    
void 
ASTForm_In::cdump()
{
  cout << "In("; t1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTForm_Notin::cdump()
{
  cout << "Notin("; t1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void
ASTForm_RootPred::cdump()
{
  cout << "Root("; t->cdump(); cout << ",["; ul->dump(); cout << ")"; 
}

void 
ASTForm_EmptyPred::cdump()
{
  cout << "EmptyPred("; T->dump(); cout << ")";
}
    
void 
ASTForm_FirstOrder::cdump()
{
  cout << "FirstOrder("; t->cdump(); cout << ")";
}
    
void 
ASTForm_Sub::cdump()
{
  cout << "Sub("; T1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTForm_Equal1::cdump()
{
  cout << "Equal1("; t1->cdump(); cout << ","; t2->cdump(); cout << ")";
}
    
void 
ASTForm_Equal2::cdump()
{
  cout << "Equal2("; T1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTForm_NotEqual1::cdump()
{
  cout << "NotEqual1("; t1->cdump(); cout << ","; t2->cdump(); cout << ")";
}
    
void 
ASTForm_NotEqual2::cdump()
{
  cout << "NotEqual2("; T1->cdump(); cout << ","; T2->cdump(); cout << ")";
}
    
void 
ASTForm_Less::cdump()
{
  cout << "Less("; t1->cdump(); cout << ","; t2->cdump(); cout << ")";
}
    
void 
ASTForm_LessEq::cdump()
{
  cout << "LessEq("; t1->cdump(); cout << ","; t2->cdump(); cout << ")";
}
    
void 
ASTForm_WellFormedTree::cdump()
{
  cout << "WellFormedTree("; T->cdump(); cout << ")";
}
    
void 
ASTForm_Impl::cdump()
{
  cout << "Impl("; f1->cdump(); cout << ","; f2->cdump(); cout << ")";
}
    
void 
ASTForm_Biimpl::cdump()
{
  cout << "Biimpl("; f1->cdump(); cout << ","; f2->cdump(); cout << ")";
}
    
void 
ASTForm_And::cdump()
{
  cout << "And("; f1->cdump(); cout << ","; f2->cdump(); cout << ")";
}
    
void 
ASTForm_IdLeft::cdump()
{
  cout << "IdLeft("; f1->cdump(); cout << ","; f2->cdump(); cout << ")";
}
    
void 
ASTForm_Or::cdump()
{
  cout << "Or("; f1->cdump(); cout << ","; f2->cdump(); cout << ")";
}
    
void 
ASTForm_Not::cdump()
{
  cout << "Not("; f->cdump(); cout << ")";
}
    
void 
ASTForm_Ex0::cdump()
{
  cout << "Ex0("; vl->dump(); cout << ","; f->cdump(); cout << ")";
}
    
void 
ASTForm_Ex1::cdump()
{

  cout << "Ex1("; vl->dump(); cout << ","; f->cdump(); cout << ")";
}
    
void 
ASTForm_Ex2::cdump()
{
  cout << "Ex2("; vl->dump(); cout << ","; f->cdump(); cout << ")";
}
    
void 
ASTForm_All0::cdump()
{
  cout << "All0("; vl->dump(); cout << ","; f->cdump(); cout << ")";
}
    
void 
ASTForm_All1::cdump()
{
  cout << "All1("; vl->dump(); cout << ","; f->cdump(); cout << ")";
}
    
void 
ASTForm_All2::cdump()
{
  cout << "All2("; vl->dump(); cout << ","; f->cdump(); cout << ")";
}
    
void 
ASTForm_Let0::cdump()
{
  IdentList::iterator ident;
  FormList::iterator form;

  cout << "Let0(";

  for (ident = defIdents->begin(), form = defForms->begin(); 
       ident != defIdents->end(); ident++, form++) {
    cout << "(Var0 " << symbolTable.lookupSymbol(*ident) 
	 << ","; (*form)->cdump(); cout << "),";
  }

  f->cdump(); cout << ")";
}
    
void 
ASTForm_Let1::cdump()
{
  IdentList::iterator ident;
  Term1List::iterator term;

  cout << "Let1(";

  for (ident = defIdents->begin(), term = defTerms->begin(); 
       ident != defIdents->end(); ident++, term++) {
    cout << "(Var1 " << symbolTable.lookupSymbol(*ident) 
      << ","; (*term)->cdump(); cout << "),";
  }

  f->cdump(); cout << ")";
}
    
void 
ASTForm_Let2::cdump()
{
  IdentList::iterator ident;
  Term2List::iterator term;

  cout << "Let2(";

  for (ident = defIdents->begin(), term = defTerms->begin(); 
       ident != defIdents->end(); ident++, term++) {
    cout << "(Var2 " << symbolTable.lookupSymbol(*ident) 
      << ","; (*term)->cdump(); cout << "),";
  }

  f->cdump(); cout << ")";
}
    
void 
ASTForm_Call::cdump()
{
  cout << "Call(" << symbolTable.lookupSymbol(n);
  args->cdump();
  cout << ")";
}

void 
ASTForm_Import::cdump()
{
  cout << "Import(\"" << file << "\"";
  Deque<char *>::iterator i;
  IdentList::iterator j;
  for (i = fileVars->begin(), j = idents->begin(); 
       i != fileVars->end(); i++, j++)
    cout << ",(" << *i << "," << symbolTable.lookupSymbol(*j) << ")";
  cout << ")";
}

void 
ASTForm_Export::cdump()
{
  cout << "Export(\"" << file << "\","; f->cdump(); cout << ")";
}

void 
ASTForm_Prefix::cdump()
{
  cout << "Prefix("; f->cdump(); cout << ")";
}

void 
ASTForm_Restrict::cdump()
{
  cout << "Restrict("; f->cdump(); cout << ")";
}

void 
ASTForm_InStateSpace1::cdump()
{
  cout << "InStateSpace1("; t->cdump(); 
  cout << ","; ss->dump(); cout << ")";
}

void 
ASTForm_InStateSpace2::cdump()
{
  cout << "InStateSpace2("; T->cdump(); 
  cout << ","; ss->dump(); cout << ")";
}

void 
ASTForm_SomeType::cdump()
{
  cout << "SomeType("; t->cdump(); cout << ")";
}


/* Transformation to restricted syntax - removal of implication and equivalence */

// Transformation: A -> B = ~A | B
ASTForm* ASTForm_Impl::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();

   ASTForm* not_f1 = new ASTForm_Not(f1, pos);
   return (ASTForm*) new ASTForm_Or(not_f1, f2, pos);
}

// Transformation: A <-> B = (~A | B) & (A | ~B)
ASTForm* ASTForm_Biimpl::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   
   ASTForm* not_f1 = new ASTForm_Not(f1, pos);
   ASTForm* not_f2 = new ASTForm_Not(f2, pos);
   ASTForm* ff1 = f1->clone();
   ASTForm* ff2 = f2->clone();
   ASTForm* impl1 = new ASTForm_Or(not_f1, ff2, pos);
   ASTForm* impl2 = new ASTForm_Or(ff1, not_f2, pos);

   return (ASTForm*) new ASTForm_And(impl1, impl2, pos);
}

ASTForm* ASTForm_IdLeft::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_Or::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_And::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_Ex0::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_Ex1::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_Ex2::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_All0::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_All1::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_All2::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

ASTForm* ASTForm_Not::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/* Transformations to Prenex Normal Form */
//  1) childs are transformed to prenex normal form 
//  2) While, there exists a child node with prenex 

bool hasQuantifier(ASTForm* child) {
   return (child->kind == aEx0) | (child->kind == aEx1) | (child->kind == aEx2) |
          (child->kind == aAll0) | (child->kind == aAll1) | (child->kind == aAll2);  
}

ASTForm* switchNodeWithQuantifier(ASTForm_vf* quantifier, ASTForm* & current, ASTForm* parent) {
    ASTForm* formula;
    formula = quantifier->f;
    quantifier->f = parent;
    if(current != 0) {
      ((ASTForm_q*)current)->f = quantifier;
    }
    current = quantifier;
    return formula;
}

ASTForm *switchNodeWithQuantifier(ASTForm_uvf* quantifier, ASTForm* & current, ASTForm* parent) {
    ASTForm* formula;
    formula = quantifier->f;
    quantifier->f = parent;
    if(current != 0) {
      ((ASTForm_q*)current)->f = quantifier;
    }
    current = quantifier;
    return formula;
}

// Binary node, does not expect implication or biimplication there!
ASTForm* ASTForm_ff::toPrenexNormalForm() {
   bool leftHasQuantifier, rightHasQuantifier;
   f1 = f1->toPrenexNormalForm();
   f2 = f2->toPrenexNormalForm();
   
   ASTForm* root, *current;
   root = 0;
   current = 0;   

   do {
       leftHasQuantifier = hasQuantifier(f1);
       rightHasQuantifier = hasQuantifier(f2);

       if(leftHasQuantifier) {
           f1 = (f1->kind == aEx0 | f1->kind == aAll0) ?
              switchNodeWithQuantifier((ASTForm_vf*)f1, current, this) :
              switchNodeWithQuantifier((ASTForm_uvf*)f1, current, this);
           root = (root == 0) ? current : root;
           continue;
       }

       if(rightHasQuantifier) {
           f2 = (f2->kind == aEx0 | f2->kind == aAll0) ?
              switchNodeWithQuantifier((ASTForm_vf*)f2, current, this) :
              switchNodeWithQuantifier((ASTForm_uvf*)f2, current, this);
           root = (root == 0) ? current : root;
       }
   } while (leftHasQuantifier | rightHasQuantifier); 
   return (root == 0) ? this : root;
}

ASTForm* negateQuantifier(ASTForm_Not* node) {
    ASTForm* formula, *q;
    q = node->f;
    switch(q->kind) {
        case aEx0:
            formula = new ASTForm_Not(((ASTForm_Ex0*)q)->f, node->pos);
            return new ASTForm_All0(((ASTForm_Ex0*)q)->vl, formula, ((ASTForm_Ex0*)q)->pos);
            break;
        case aEx1:
            formula = new ASTForm_Not(((ASTForm_Ex1*)q)->f, node->pos);
            return new ASTForm_All1(((ASTForm_Ex1*)q)->ul, ((ASTForm_Ex1*)q)->vl, formula, ((ASTForm_Ex1*)q)->pos);
            break;
        case aEx2:
            formula = new ASTForm_Not(((ASTForm_Ex2*)q)->f, node->pos);
            return new ASTForm_All2(((ASTForm_Ex2*)q)->ul, ((ASTForm_Ex2*)q)->vl, formula, ((ASTForm_Ex2*)q)->pos);
            break;
        case aAll0:
            formula = new ASTForm_Not(((ASTForm_All0*)q)->f, node->pos);
            return new ASTForm_Ex0(((ASTForm_All0*)q)->vl, formula, ((ASTForm_All0*)q)->pos);
            break;
        case aAll1:
            formula = new ASTForm_Not(((ASTForm_All1*)q)->f, node->pos);
            return new ASTForm_Ex1(((ASTForm_All1*)q)->ul, ((ASTForm_All1*)q)->vl, formula, ((ASTForm_All1*)q)->pos);
            break;
        case aAll2:
            formula = new ASTForm_Not(((ASTForm_All2*)q)->f, node->pos);
            return new ASTForm_Ex2(((ASTForm_All2*)q)->ul, ((ASTForm_All2*)q)->vl, formula, ((ASTForm_All2*)q)->pos);
            break;
        default:
            return node;
    }
}
// Application of following transformations:
// not(ex fi) = all(not fi)
// not(all fi) = ex(not fi)

ASTForm* ASTForm_Not::toPrenexNormalForm() {
    f = f->toPrenexNormalForm();
    ASTForm* temp;
    return negateQuantifier(this);
}


ASTForm* ASTForm_vf::toPrenexNormalForm() {
    f = f->toPrenexNormalForm();
    return this;
}

ASTForm* ASTForm_uvf::toPrenexNormalForm() {
    f = f->toPrenexNormalForm();
    return this;
}

/* Transformation - Removal of universal quantifier */
// Aplication of following transformation: forall fi = not exists not fi
ASTForm* ASTForm_All0::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    ASTForm *negFi, *exNegFi, *formula;
    negFi = new ASTForm_Not(f, pos);
    exNegFi = new ASTForm_Ex0(vl, negFi, pos);
    formula = new ASTForm_Not(exNegFi, pos);
    return formula;
}

ASTForm* ASTForm_All1::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    ASTForm *negFi, *exNegFi, *formula;
    negFi = new ASTForm_Not(f, pos);
    exNegFi = new ASTForm_Ex1(ul, vl, negFi, pos);
    formula = new ASTForm_Not(exNegFi, pos);
    return formula;
}

ASTForm* ASTForm_All2::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    ASTForm *negFi, *exNegFi, *formula;
    negFi = new ASTForm_Not(f, pos);
    exNegFi = new ASTForm_Ex2(ul, vl, negFi, pos);
    formula = new ASTForm_Not(exNegFi, pos);
    return formula;
}

ASTForm* ASTForm_Not::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    return this;
}

ASTForm* ASTForm_ff::removeUniversalQuantifier() {
    f1 = f1->removeUniversalQuantifier();
    f2 = f2->removeUniversalQuantifier();
    return this;
}

ASTForm* ASTForm_vf::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    return this;
}

ASTForm* ASTForm_uvf::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    return this;
}

/* Transformation: Unfolding negations */
// Following trasnformations are applied:
ASTForm* ASTForm_Not::unfoldNegations() {
    f = f->unfoldNegations();
    switch(f->kind) {
    //  not not A = A
        case aNot:
	    return ((ASTForm_Not*)f)->f;
    //  not (A or B) = not A and not B
        case aOr:
            ASTForm_Or *child1;
            ASTForm *l1, *r1;
            child1 = (ASTForm_Or*) f;
            l1 = (ASTForm*) new ASTForm_Not(child1->f1, pos);
            r1 = (ASTForm*) new ASTForm_Not(child1->f2, pos);
            return new ASTForm_And(l1, r1, pos);
    //  not (A and B) = not A or not B
        case aAnd:
            ASTForm_And *child2;
            ASTForm *l2, *r2;
            child2 = (ASTForm_And*) f;
            l2 = (ASTForm*) new ASTForm_Not(child2->f1, pos);
            r2 = (ASTForm*) new ASTForm_Not(child2->f2, pos);
            return new ASTForm_Or(l2, r2, pos);
	default:
	    return this;
    }
}

ASTForm* ASTForm_ff::unfoldNegations() {
    f1 = f1->unfoldNegations();
    f2 = f2->unfoldNegations();
    return this;
}

ASTForm* ASTForm_vf::unfoldNegations() {
    f = f->unfoldNegations();
    return this;
}

ASTForm* ASTForm_uvf::unfoldNegations() {
    f = f->unfoldNegations();
    return this;
} 

ASTForm* ASTForm::toExistentionalPNF() {
   ASTForm* temp;
   temp = this->toRestrictedSyntax();
   temp = temp->toPrenexNormalForm();
   temp = temp->removeUniversalQuantifier();
   return temp->unfoldNegations();
}

