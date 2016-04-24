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

#include <typeinfo>
#include "ast.h"
#include "symboltable.h"
#include "../DecisionProcedure/environment.hh"

using std::cout;

extern SymbolTable symbolTable;

void 
ASTComponent::dump()
{
  cout << name;
  if (path)
    path->dump();
  else
    cout << "[?]";
  cout << ":" << type;
}

void
ASTComponentList::dump()
{
  for (iterator i = begin(); i != end(); i++) {
    if (i != begin())
      cout << ",";
    (*i)->dump();
  }
}

void
ASTVariant::dump()
{
  cout << name;
  if (path)
    path->dump();
  else
    cout << "[?]";
  cout << "(";
  if (components)
    components->dump();
  cout << ")";
}

void
ASTVariantList::dump()
{
  for (iterator i = begin(); i != end(); i++) {
    if (i != begin())
      cout << ",";
    (*i)->dump();
  }
}

void
BitList::dump()
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
ASTList::dump()
{
  iterator i;
  for (i = begin(); i != end(); i++) {
    cout << ","; (*i)->dump();
  }
}

void 
ASTUniv::dump()
{
  cout << symbolTable.lookupSymbol(u);
}

void 
ASTTerm1_Var1::dump()
{
  #if (PRINT_PRETTY == true)
  cout <<  n << ":" << symbolTable.lookupSymbol(n) << "\u00B9";
  #else
  cout << "Var1 " << symbolTable.lookupSymbol(n);
  #endif
}

std::string ASTTerm1_Var1::ToString(bool no_utf) {
  if(no_utf) {
    std::string s = std::string(symbolTable.lookupSymbol(n));
    if (s[0] == '<')
      return s.substr(1, s.length()-2);
    else
      return s;
  } else {
    return (std::string(symbolTable.lookupSymbol(n)) + "\u00B9");
  }
}
  
void 
ASTTerm1_Dot::dump()
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
ASTTerm1_Up::dump()
{
  cout << "Up("; t->dump(); cout << ")";
}
    
void
ASTTerm1_Root::dump()
{
  if (univ == -1)
    cout << "Root";
  else
    cout << "Root(" << symbolTable.lookupSymbol(univ) << ")";
}

void 
ASTTerm1_Int::dump()
{
  #if (PRINT_PRETTY == true)
  cout << n;
  #else
  cout << "Int " << n;
  #endif
}

std::string ASTTerm1_Int::ToString(bool no_utf) {
  return std::to_string(n);
}
    
void 
ASTTerm1_Plus::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t->dump(); cout << " +\u00B9 " << n << ")";
  #else
  cout << "Plus1("; t->dump(); cout << "," << n << ")";
  #endif
}

std::string ASTTerm1_Plus::ToString(bool no_utf) {
  if(no_utf) {
    return "(" + t->ToString(no_utf) + " + " + std::to_string(n) + ")";
  } else {
    return ("(" + t->ToString(no_utf) + " +\u00B9 " + std::to_string(n) + ")");
  }
}
    
void 
ASTTerm1_Minus::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t->dump(); cout << " -\u00B9 " << n << ")";
  #else
  cout << "Minus1("; t->dump(); cout << "," << n << ")";
  #endif
}

std::string ASTTerm1_Minus::ToString(bool no_utf) {
  if(no_utf) {
    return "(" + t->ToString(no_utf) + " - " + std::to_string(n) + ")";
  } else {
    return ("(" + t->ToString(no_utf) + " -\u00B9 " + std::to_string(n) + ")");
  }
}
    
void 
ASTTerm1_PlusModulo::dump()
{
  cout << "PlusModulo1("; t1->dump(); cout << ","  << n << ",";
  t2->dump(); cout << ")";
}
    
void 
ASTTerm1_MinusModulo::dump()
{
  cout << "MinusModulo1("; t1->dump(); cout << "," << n << ",";
  t2->dump(); cout << ")";
}
    
void 
ASTTerm1_Min::dump()
{
  cout << "Min("; T->dump(); cout << ")";
}
    
void 
ASTTerm1_Max::dump()
{
  cout << "Max("; T->dump(); cout << ")";
}
    
void 
ASTTerm1_TreeRoot::dump()
{
  cout << "TreeRoot("; T->dump(); cout << ")";
}
    
void 
ASTTerm2_Var2::dump()
{
  #if (PRINT_PRETTY == true)
  cout << n << ":" << symbolTable.lookupSymbol(n) << "\u00B2";
  #else
  cout << "Var2 " << symbolTable.lookupSymbol(n);
  #endif
}

std::string ASTTerm2_Var2::ToString(bool no_utf) {
  if(no_utf) {
    std::string s = std::string(symbolTable.lookupSymbol(n));
    if (s[0] == '<')
      return s.substr(1, s.length()-2);
    else
      return s;
  } else {
    return (std::string(symbolTable.lookupSymbol(n)) + "\u00B2");
  }
}
    
void 
ASTTerm2_VarTree::dump()
{
  cout << "Tree " << symbolTable.lookupSymbol(n);
}
    
void 
ASTTerm2_Dot::dump()
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
ASTTerm2_Up::dump()
{
  cout << "Up("; T->dump(); cout << ")";
}
    
void 
ASTTerm2_Empty::dump()
{
  cout << "Empty";
}

std::string ASTTerm2_Empty::ToString(bool no_utf) {
  if(no_utf) {
    return "Empty";
  } else {
    return std::string("\u2205");
  }
}
    
void 
ASTTerm2_Union::dump()
{
  cout << "Union("; T1->dump(); cout << ","; T2->dump(); cout << ")";
}
    
void 
ASTTerm2_Inter::dump()
{
  cout << "Inter("; T1->dump(); cout << ","; T2->dump(); cout << ")";
}
    
void 
ASTTerm2_Setminus::dump()
{
  cout << "Setminus("; T1->dump(); cout << ","; T2->dump(); cout << ")";
}
    
void 
ASTTerm2_Set::dump()
{
  ASTList::iterator i;
  cout << "Set(";
  for (i = elements->begin(); i != elements->end();) {
    (*i)->dump();
    if (++i != elements->end()) 
      cout << ",";
  }
  cout << ")";
}
    
void 
ASTTerm2_Plus::dump()
{
  #if (PRINT_PRETTY == true)
  cout << ""; T->dump(); cout << " +\u00B2 " << n << "";
  #else
  cout << "Plus2("; T->dump(); cout << "," << n << ")";
  #endif
}

std::string ASTTerm2_Plus::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + T->ToString(no_utf) + " + " + std::to_string(n) + ")");
  } else {
    return ("(" + T->ToString(no_utf) + " +\u00B2 " + std::to_string(n) + ")");
  }
}
    
void 
ASTTerm2_Minus::dump()
{
  #if (PRINT_PRETTY == true)
  cout << ""; T->dump(); cout << " -\u00B2 " << n << "";
  #else
  cout << "Minus2("; T->dump(); cout << "," << n << ")";
  #endif
}

std::string ASTTerm2_Minus::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + T->ToString(no_utf) + " - " + std::to_string(n) + ")");
  } else {
    return ("(" + T->ToString(no_utf) + " -\u00B2 " + std::to_string(n) + ")");
  }

}
    
void 
ASTTerm2_Interval::dump()
{
  cout << "Interval("; t1->dump(); cout << ","; t2->dump(); cout << ")";
}
    
void 
ASTTerm2_PresbConst::dump()
{
  cout << "PresbConst(" << value << ")"; 
}

void 
ASTTerm2_Formula::dump()
{
  cout << "Term2Formula(" << symbolTable.lookupSymbol(fresh) << ",";
  f->dump(); cout << ")";
}

void 
ASTForm_Var0::dump()
{
  cout << "Var0 " << symbolTable.lookupSymbol(n);
}

void
ASTForm_AllPosVar::dump() {
  cout << "AllPos($)";
}

void 
ASTForm_True::dump()
{
  cout << "True";
}
    
void 
ASTForm_False::dump()
{
  cout << "False";
}
    
void 
ASTForm_In::dump()
{

  #if (PRINT_PRETTY == true)
  cout << "("; t1->dump(); cout << " \u2208 "; T2->dump(); cout  << ")";
  #else
  cout << "In("; t1->dump(); cout << ","; T2->dump(); cout << ")";
  #endif
}

std::string ASTForm_In::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + t1->ToString(no_utf) + " in " + T2->ToString(no_utf) + ")");
  } else {
    return ("(" + t1->ToString(no_utf) + " \u2208 " + T2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_Notin::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t1->dump(); cout << " \u2209 "; T2->dump(); cout << ")";
  #else
  cout << "Notin("; t1->dump(); cout << ","; T2->dump(); cout << ")";
  #endif
}

std::string ASTForm_Notin::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + t1->ToString(no_utf) + " notin " + T2->ToString(no_utf) + ")");
  } else {
    return ("(" + t1->ToString(no_utf) + " \u2209 " + T2->ToString(no_utf) + ")");
  }
}
    
void
ASTForm_RootPred::dump()
{
  cout << "Root("; t->dump(); cout << ",["; ul->dump(); cout << ")"; 
}

void 
ASTForm_EmptyPred::dump()
{
  cout << "EmptyPred("; T->dump(); cout << ")";
}
    
void 
ASTForm_FirstOrder::dump()
{
  cout << "FirstOrder("; t->dump(); cout << ")";
}

std::string ASTForm_FirstOrder::ToString(bool no_utf) {
  if(no_utf) {
    return "FirstOrder(" + t->ToString(no_utf) + ")";
  } else {
    return ("(FO (" + t->ToString(no_utf) + "))");
  }
}
    
void 
ASTForm_Sub::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; T1->dump(); cout << " \u2286 "; T2->dump(); cout << ")";
  #else
  cout << "Sub("; T1->dump(); cout << ","; T2->dump(); cout << ")";
  #endif
}

std::string ASTForm_Sub::ToString(bool no_utf) {
  if(no_utf) {
    return "(" + T1->ToString(no_utf) + " sub " + T2->ToString(no_utf) + ")";
  } else {
    return ("(" + T1->ToString(no_utf) + " \u2286 " + T2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_Equal1::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t1->dump(); cout << " \u003D\u00B9 "; t2->dump(); cout << ")";
  #else
  cout << "Equal1("; t1->dump(); cout << ","; t2->dump(); cout << ")";
  #endif
}

std::string ASTForm_Equal1::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + t1->ToString(no_utf) + " = " + t2->ToString(no_utf) + ")");
  } else {
    return ("(" + t1->ToString(no_utf) + " \u003D\u00B9 " + t2->ToString(no_utf) + ")");
  }
}

void 
ASTForm_Equal2::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; T1->dump(); cout << " \u003D\u00B2 "; T2->dump(); cout << ")";
  #else
  cout << "Equal2("; T1->dump(); cout << ","; T2->dump(); cout << ")";
  #endif
}

std::string ASTForm_Equal2::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + T1->ToString(no_utf) + " = " + T2->ToString(no_utf) + ")");
  } else {
    return ("(" + T1->ToString(no_utf) + " \u003D\u00B2 " + T2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_NotEqual1::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t1->dump(); cout << " \u2260\u00B9 "; t2->dump(); cout << ")";
  #else
  cout << "NotEqual1("; t1->dump(); cout << ","; t2->dump(); cout << ")";
  #endif
}

std::string ASTForm_NotEqual1::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + t1->ToString(no_utf) + " ~= " + t2->ToString(no_utf) + ")");
  } else {
    return ("(" + t1->ToString(no_utf) + " \u2260\u00B9 " + t2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_NotEqual2::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; T1->dump(); cout << " \u2260\u00B2 "; T2->dump(); cout << ")";
  #else
  cout << "NotEqual2("; T1->dump(); cout << ","; T2->dump(); cout << ")";
  #endif
}

std::string ASTForm_NotEqual2::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + T1->ToString(no_utf) + " ~= " + T2->ToString(no_utf) + ")");
  } else {
    return ("(" + T1->ToString(no_utf) + " \u2260\u00B2 " + T2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_Less::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t1->dump(); cout << " \u003C\u00B9 "; t2->dump(); cout << ")";
  #else
  cout << "Less("; t1->dump(); cout << ","; t2->dump(); cout << ")";
  #endif
}

std::string ASTForm_Less::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + t1->ToString(no_utf) + " < " + t2->ToString(no_utf) + ")");
  } else {
    return ("(" + t1->ToString(no_utf) + " \u003C\u00B9 " + t2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_LessEq::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; t1->dump(); cout << " \u2264\u00B9 "; t2->dump(); cout << ")";
  #else
  cout << "LessEq("; t1->dump(); cout << ","; t2->dump(); cout << ")";
  #endif
}

std::string ASTForm_LessEq::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + t1->ToString(no_utf) + " <= " + t2->ToString(no_utf) + ")");
  } else {
    return ("(" + t1->ToString(no_utf) + " \u2264\u00B9 " + t2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_WellFormedTree::dump()
{
  cout << "WellFormedTree("; T->dump(); cout << ")";
}
    
void 
ASTForm_Impl::dump()
{
  cout << "("; f1->dump(); cout << " => "; f2->dump(); cout << ")";
}

std::string ASTForm_Impl::ToString(bool no_utf) {
  return ("(" + f1->ToString(no_utf) + " => " + f2->ToString(no_utf) + ")");
}
    
void 
ASTForm_Biimpl::dump()
{
  cout << "("; f1->dump(); cout << " <=> "; f2->dump(); cout << ")";
}

std::string ASTForm_Biimpl::ToString(bool no_utf) {
  return ("(" + f1->ToString(no_utf) + " <=> " + f2->ToString(no_utf) + ")");
}
    
void 
ASTForm_And::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; f1->dump(); cout << " \u2227 "; f2->dump(); cout << ")";
  #else
  cout << "("; f1->dump(); cout << " & "; f2->dump(); cout << ")";
  #endif
}

std::string ASTForm_And::ToString(bool no_utf) {
  if(no_utf) {
    return "(" + f1->ToString(no_utf) + " & " + f2->ToString(no_utf) + ")";
  } else {
    return ("(" + f1->ToString(no_utf) + " \u2227 " + f2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_IdLeft::dump()
{
  cout << "IdLeft("; f1->dump(); cout << ","; f2->dump(); cout << ")";
}
    
void 
ASTForm_Or::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "("; f1->dump(); cout << " \u2228 "; f2->dump(); cout << ")";
  #else
  cout << "("; f1->dump(); cout << " | "; f2->dump(); cout << ")";
  #endif
}

std::string ASTForm_Or::ToString(bool no_utf) {
  if(no_utf) {
    return ("(" + f1->ToString(no_utf) + " | " + f2->ToString(no_utf) + ")");
  } else {
    return ("(" + f1->ToString(no_utf) + " \u2228 " + f2->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_Not::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u00AC("; f->dump(); cout << ")";
  #else
  cout << "~("; f->dump(); cout << ")";
  #endif
}

std::string ASTForm_Not::ToString(bool no_utf) {
  if(no_utf) {
    return "~" + f->ToString(no_utf);
  } else {
    return ("(\u00AC " + f->ToString(no_utf) + ")");
  }
}
    
void 
ASTForm_Ex0::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u22030("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #else
  cout << "Ex0("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #endif
}
    
void 
ASTForm_Ex1::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u22031("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #else
  cout << "Ex1("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #endif
}

std::string quantifier_to_string(IdentList* free, std::string quantifier) {
  std::string s(quantifier);
  std::string var("");
  s += " ";
  bool first = true;
  for (auto it = free->begin(); it != free->end(); ++it) {
    if (first) {
      first = false;
    } else {
      s += ", ";
    }
    var = std::string(symbolTable.lookupSymbol(*it));
    s += (var[0] == '<' ? var.substr(1, var.length()-2) : var);
  }
  s += ": ";
  return s;
}
std::string ASTForm_Ex1::ToString(bool no_utf) {
  return "(" +  quantifier_to_string(this->vl, "ex1") + this->f->ToString(no_utf) + ")";
}
    
void 
ASTForm_Ex2::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u22032("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #else
  cout << "Ex2("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #endif
}

std::string ASTForm_Ex2::ToString(bool no_utf) {
  return "(" +  quantifier_to_string(this->vl, "ex2") + this->f->ToString(no_utf) + ")";
}
    
void 
ASTForm_All0::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u22000("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #else
  cout << "All0("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #endif
}
    
void 
ASTForm_All1::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u22001("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #else
  cout << "All1("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #endif
}

std::string ASTForm_All1::ToString(bool no_utf) {
  return "(" + quantifier_to_string(this->vl, "all1") + this->f->ToString(no_utf) + ")";
}
    
void 
ASTForm_All2::dump()
{
  #if (PRINT_PRETTY == true)
  cout << "\u22002("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #else
  cout << "All2("; vl->dump(); cout << ","; f->dump(); cout << ")";
  #endif
}

std::string ASTForm_All2::ToString(bool no_utf) {
  return "(" + quantifier_to_string(this->vl, "all2") + this->f->ToString(no_utf) + ")";
}
    
void 
ASTForm_Let0::dump()
{
  IdentList::iterator ident;
  FormList::iterator form;

  cout << "Let0(";

  for (ident = defIdents->begin(), form = defForms->begin(); 
       ident != defIdents->end(); ident++, form++) {
    cout << "(Var0 " << symbolTable.lookupSymbol(*ident) 
	 << ","; (*form)->dump(); cout << "),";
  }

  f->dump(); cout << ")";
}
    
void 
ASTForm_Let1::dump()
{
  IdentList::iterator ident;
  Term1List::iterator term;

  cout << "Let1(";

  for (ident = defIdents->begin(), term = defTerms->begin(); 
       ident != defIdents->end(); ident++, term++) {
    cout << "(Var1 " << symbolTable.lookupSymbol(*ident) 
      << ","; (*term)->dump(); cout << "),";
  }

  f->dump(); cout << ")";
}
    
void 
ASTForm_Let2::dump()
{
  IdentList::iterator ident;
  Term2List::iterator term;

  cout << "Let2(";

  for (ident = defIdents->begin(), term = defTerms->begin(); 
       ident != defIdents->end(); ident++, term++) {
    cout << "(Var2 " << symbolTable.lookupSymbol(*ident) 
      << ","; (*term)->dump(); cout << "),";
  }

  f->dump(); cout << ")";
}
    
void 
ASTForm_Call::dump()
{
  cout << "Call(" << symbolTable.lookupSymbol(n);
  args->dump();
  cout << ")";
}

void 
ASTForm_Import::dump()
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
ASTForm_Export::dump()
{
  cout << "Export(\"" << file << "\","; f->dump(); cout << ")";
}

void 
ASTForm_Prefix::dump()
{
  cout << "Prefix("; f->dump(); cout << ")";
}

void 
ASTForm_Restrict::dump()
{
  cout << "Restrict("; f->dump(); cout << ")";
}

void 
ASTForm_InStateSpace1::dump()
{
  cout << "InStateSpace1("; t->dump(); 
  cout << ","; ss->dump(); cout << ")";
}

void 
ASTForm_InStateSpace2::dump()
{
  cout << "InStateSpace2("; T->dump(); 
  cout << ","; ss->dump(); cout << ")";
}

void 
ASTForm_SomeType::dump()
{
  cout << "SomeType("; t->dump(); cout << ")";
}
