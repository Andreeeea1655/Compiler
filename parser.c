#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "parser.h"
#include "utils.h"
#include "ad.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token
Symbol *owner=NULL;		// the owner of the symbols from the current domain

//forward declarations for all the non-terminals in the grammar
bool unit();
bool structDef();
bool varDef();
bool typeBase(Type* t);
bool arrayDecl(Type* t);
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound(bool newDomain);
bool expr();
bool exprAssign();
bool exprOr();
bool exprOrPrim();
bool exprAnd();
bool exprAndPrim();
bool exprEq();
bool exprEqPrim();
bool exprRel();
bool exprRelPrim();
bool exprAdd();
bool exprAddPrim();
bool exprMul();
bool exprMulPrim();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPostfixPrim();
bool exprPrimary();

void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
}

const char* tkCodeName(int code){
	switch(code){
		case ID:return "ID";
		case INT:return "INT";
		case DOUBLE:return "DOUBLE";
		case CHAR:return "CHAR";		
		case STRING:return "STRING";
		case TYPE_CHAR:return "TYPE_CHAR";
		case TYPE_DOUBLE:return "TYPE_DOUBLE";
		case TYPE_INT:return "TYPE_INT";
		case IF:return "IF";
		case ELSE:return "ELSE";
		case WHILE:return "WHILE";
		case RETURN:return "RETURN";
		case VOID:return "VOID";	
		case STRUCT:return "STRUCT";
		//delimiters
		case COMMA:return "COMMA";
		case SEMICOLON:return "SEMICOLON";
		case LPAR:return "LPAR";
		case RPAR:return "RPAR";
		case LBRACKET:return "LBRACKET";
		case RBRACKET:return "RBRACKET";
		case LACC:return "LACC";
		case RACC:return "RACC";
		case END:return "END";
		//operators
		case ADD:return "ADD";
		case SUB:return "SUB";
		case MUL:return "MUL";
		case DIV:return "DIV";
		case DOT:return "DOT";
		case AND:return "AND";
		case OR:return "OR";
		case NOT:return "NOT";
		case ASSIGN:return "ASSIGN";
		case EQUAL:return "EQUAL";
		case NOTEQ:return "NOTEQ";
		case LESS:return "LESS";
		case LESSEQ:return "LESSEQ";
		case GREATER:return "GREATER";
		case GREATEREQ:return "GREATEREQ";
		default: return "UNKNOWN TOKEN";
	}
}

bool consume(int code){
	//printf("consume(%s)",tkCodeName(code));
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		//printf(" => consumed\n");
		return true;
	}
	//printf(" => found %s\n",tkCodeName(iTk->code));
	return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit(){
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
	}
	return false;
}
// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef(){
	Token* start=iTk;
	if(consume(STRUCT)){
		if(consume(ID)){
			Token* tkName=consumedTk;
			if(consume(LACC)){
				Symbol *s=findSymbolInDomain(symTable,tkName->text); 
				if(s)tkerr("symbol redefinition: %s",tkName->text); 
				s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT)); 
				s->type.tb=TB_STRUCT; 
				s->type.s=s; 
				s->type.n=-1; 
				pushDomain(); 
				owner=s;
				while(varDef()){}
				if(consume(RACC)){
					if(consume(SEMICOLON)){
						owner=NULL; 
						dropDomain();
						return true;
					}else tkerr("syntax error: missing ';'");
				}else tkerr("syntax error: missing '}'");
			}
		}
	}
	iTk=start;
	return false;
}
// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef(){
	Token* start=iTk;
	Type t;
	if(typeBase(&t)){
		if(consume(ID)){
			Token* tkName=consumedTk;
			if(arrayDecl(&t)){
				if(t.n==0)tkerr("a vector variable must have a specified dimension");
			}

			if(consume(SEMICOLON)){
				Symbol *var=findSymbolInDomain(symTable,tkName->text); 
				if(var)tkerr("symbol redefinition: %s",tkName->text); 
				var=newSymbol(tkName->text,SK_VAR); 
				var->type=t; 
				var->owner=owner; 
				addSymbolToDomain(symTable,var); 
				if(owner){ 
					switch(owner->kind){ 
						case SK_FN: 
							var->varIdx=symbolsLen(owner->fn.locals); 
							addSymbolToList(&owner->fn.locals,dupSymbol(var)); 
							break; 
						case SK_STRUCT: 
							var->varIdx=typeSize(&owner->type); 
							addSymbolToList(&owner->structMembers,dupSymbol(var)); 
							break; 
						} 
				}else{ 
					var->varMem=safeAlloc(typeSize(&t)); 
				}
				return true;
			}else tkerr("syntax error: expected ';' at the end of variable definition");
		}else tkerr("syntax error: expected variable name");
	}
	iTk=start;
	return false;
}
// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(Type* t){
	t->n=-1; //not an array
	if(consume(TYPE_INT)){
		t->tb=TB_INT;
		return true;
		}
	if(consume(TYPE_DOUBLE)){
		t->tb=TB_DOUBLE;
		return true;
		}
	if(consume(TYPE_CHAR)){
		t->tb=TB_CHAR;
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID)){
			Token* tkName=consumedTk;
			t->tb=TB_STRUCT; 
			t->s=findSymbol(tkName->text); 
			if(!t->s)tkerr("structura nedefinita: %s",tkName->text);
			return true;
			}else tkerr("syntax error: expected struct name");
		}
	return false;
}
//arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl(Type* t){
	if(consume(LBRACKET)){
		if(consume(INT)){
			Token *tkSize=consumedTk;
			t->n=tkSize->i;
		}else
			t->n=0; //optional array size     
		if(consume(RBRACKET)){
			return true;
		}else tkerr("syntax error: expected ']' in array declaration");
	}
	return false;
}
/* fnDef: ( typeBase | VOID ) ID 
	LPAR ( fnParam ( COMMA fnParam )* )? RPAR 
	stmCompound*/
bool fnDef(){
	Token* start=iTk;
	Type t;
	if(typeBase(&t)){}
	else if(consume(VOID)){t.tb=TB_VOID;}
	else{ iTk=start; return false;}
		if(consume(ID)){
			Token* tkName=consumedTk;
			if(consume(LPAR)){
				Symbol *fn=findSymbolInDomain(symTable,tkName->text); 
				if(fn)tkerr("symbol redefinition: %s",tkName->text); 
				fn=newSymbol(tkName->text,SK_FN); fn->type=t; 
				addSymbolToDomain(symTable,fn); 
				owner=fn; 
				pushDomain();
				if(fnParam()){
					while(consume(COMMA)){
						if(!fnParam()){
							tkerr("syntax error: missing or invalid parameter after ','");
						}
					}
				}
				if(consume(RPAR)){
					if(stmCompound(false)){
						dropDomain(); 
						owner=NULL;
						return true;
					}else tkerr("syntax error: expected function body");
				}else tkerr("syntax error: expected ')' after function parameters");
			}
		}
	iTk=start;
	return false;
}
// fnParam: typeBase ID arrayDecl?
bool fnParam(){
	Token* start=iTk;
	Type t;
	if(typeBase(&t)){
		if(consume(ID)){
			Token* tkName=consumedTk;
			if(arrayDecl(&t)){
				t.n=0;
			}
			Symbol *param=findSymbolInDomain(symTable,tkName->text); 
			if(param)tkerr("symbol redefinition: %s",tkName->text); 
			param=newSymbol(tkName->text,SK_PARAM); 
			param->type=t; param->owner=owner; param->paramIdx=symbolsLen(owner->fn.params); 
			// parametrul este adaugat atat la domeniul curent, cat si la parametrii fn 
			addSymbolToDomain(symTable,param); 
			addSymbolToList(&owner->fn.params,dupSymbol(param));
			return true;
		}else tkerr("syntax error: expected parameter name");
	}
	iTk=start;
	return false;
}
/* stm: stmCompound
 	| IF LPAR expr RPAR stm ( ELSE stm )? 
	| WHILE LPAR expr RPAR stm 
	| RETURN expr? SEMICOLON 
	| expr? SEMICOLON*/
bool stm(){
	Token* start=iTk;
	if(stmCompound(true)){
		return true;
	}
	if(consume(IF)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						if(consume(ELSE)){
							if(!stm()) tkerr("syntax error: expected instruction");
						}
						return true;
					}else tkerr("syntax error: expected instruction after 'if' condition");
				}else tkerr("syntax error: expected ')' after 'if' condition");
			}else tkerr("syntax error: expected expression in 'if' condition");
		}else tkerr("syntax error: expected '(' after 'if'");
	}
	if(consume(WHILE)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						return true;					
					}
				}else tkerr("syntax error: expected ')' after 'while' condition");
			}else tkerr("syntax error: expected expression in 'while' condition");
		}else tkerr("syntax error: expected '(' after 'while'");
	}
	if(consume(RETURN)){
		expr();
		if(consume(SEMICOLON)){
			return true;
		}else tkerr("syntax error: expected ';' after return statement");
	}
	if(expr()){
		if(consume(SEMICOLON)){
			return true;
		}else tkerr("syntax error: expected ';' after expression");
	}
	if(consume(SEMICOLON)){
		return true; //pt ;
	}
	iTk=start;
	return false;
}
// stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(bool newDomain){
	Token* start=iTk;
	if(consume(LACC)){
		if(newDomain){
			pushDomain();
		}
		for(;;){
			if(varDef()){}
			else if(stm()){}
			else break;
		}
		if(consume(RACC)){
			if(newDomain){
				dropDomain();
			}
			return true;
		}else tkerr("syntax error: expected '}'");
	}
	iTk=start;
	return false;
}
// expr: exprAssign
bool expr(){
	if(exprAssign()){
		return true;
	}
	return false;
}
// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign(){
	Token* start=iTk;
	if(exprUnary()){
		if(consume(ASSIGN)){
			if(exprAssign()){
				return true;
			}else tkerr("syntax error: expected expression after '='");
		}
	}
	iTk=start;
	if(exprOr()){
		return true;
	}
	return false;
}
/* exprOr: exprOr OR exprAnd | exprAnd
		=> exprOr: exprAnd exprOrPrim
		=> exprOrPrim: OR exprAnd exprOrPrim | epsilon
*/
bool exprOr(){
	if(exprAnd()){
		if(exprOrPrim()){
			return true;
		}
	}
	return false;
}
bool exprOrPrim(){
	if(consume(OR)){
		if(exprAnd()){
			if(exprOrPrim()){
				return true;
			}else tkerr("syntax error: expected expression after '||'");
		}
	}
	return true; //epsilon
}
/*exprAnd: exprAnd AND exprEq | exprEq
	=> exprAnd: exprEq exprAndPrim
	=> exprAndPrim: AND exprEq exprAndPrim | epsilon
*/
bool exprAnd(){
	if(exprEq()){
		if(exprAndPrim()){
			return true;
		}
	}
	return false;
}
bool exprAndPrim(){
	if(consume(AND)){
		if(exprEq()){
			if(exprAndPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '&&'");
	}
	return true;
}
/* exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
	=> exprEq: exprRel exprEqPrim
	=> exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | epsilon
*/
bool exprEq(){
	if(exprRel()){
		if(exprEqPrim()){
			return true;
		}
	}
	return false;
}
bool exprEqPrim(){
	if(consume(EQUAL)){
		if(exprRel()){
			if(exprEqPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '=='");
	}
	if(consume(NOTEQ)){
		if(exprRel()){
			if(exprEqPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '!='");
	}
	return true;
}
/*exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
	=> exprRel: exprAdd exprRelPrim
    => exprRelPrim: (LESS | LESSEQ | GREATER | GREATEREQ) exprAdd exprRelPrim | ε
*/
bool exprRel(){
	if(exprAdd()){
		if(exprRelPrim()){
			return true;
		}
	}
	return false;
}
bool exprRelPrim(){
	if(consume(LESS)){
		if(exprAdd()){
			if(exprRelPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '<'");
	}
	if(consume(LESSEQ)){
		if(exprAdd()){
			if(exprRelPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '<=");
	}
	if(consume(GREATER)){
		if(exprAdd()){
			if(exprRelPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '>");
	}
	if(consume(GREATEREQ)){
		if(exprAdd()){
			if(exprRelPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '>=");
	}
	return true;
}
/* exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
	=> exprAdd: exprMul exprAddPrim
	=> exprAddPrim: (ADD | SUB) exprMul exprAddPrim | ε
*/
bool exprAdd(){
	if(exprMul()){
		if(exprAddPrim()){
			return true;
		}
	}
	return false;
}
bool exprAddPrim(){
	if(consume(ADD) || consume(SUB)){
		if(exprMul()){
			if(exprAddPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '+' or '-'");
	}
	return true;
}
/* exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
	=> exprMul: exprCast exprMulPrim
	=> exprMulPrim: (MUL | DIV) exprCast exprMulPrim | ε
*/
bool exprMul(){
	if(exprCast()){
		if(exprMulPrim()){
			return true;
		}
	}
	return false;
}
bool exprMulPrim(){
	if(consume(MUL)){
		if(exprCast()){
			if(exprMulPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '*'");
	}
	if(consume(DIV)){
		if(exprCast()){
			if(exprMulPrim()){
				return true;
			}
		}else tkerr("syntax error: expected expression after '/'");
	}
	return true;
}
// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast(){
	Token* start=iTk;
	if(consume(LPAR)){
		Type t;
		if(typeBase(&t)){
			arrayDecl(&t);
			if(consume(RPAR)){
				if(exprCast()){
					return true;
				}else tkerr("syntax error: expected expression after cast");
			}else tkerr("syntax error: expected ')'");
		}else tkerr("syntax error: expected type in cast");
	}
	iTk=start;
	if(exprUnary()){
		return true;
	}
	return false;
}
// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary(){
	Token* start=iTk;
	if(consume(SUB)){
		if(exprUnary()){
			return true;
		}else tkerr("syntax error: expected expression after '-'");
	}
	if(consume(NOT)){
		if(exprUnary()){
			return true;
		}else tkerr("syntax error: expected expression after '!'");
	}
	iTk=start;
	if(exprPostfix()){
		return true;
	}
	return false;
} 
/* exprPostfix: exprPostfix LBRACKET expr RBRACKET  - ?
	| exprPostfix DOT ID  
	| exprPrimary
	=> 
exprPostfix: exprPrimary exprPostfixPrim
exprPostfixPrim: 
    LBRACKET expr RBRACKET exprPostfixPrim
    | DOT ID exprPostfixPrim
    | epsilon
*/
bool exprPostfix(){
	if(exprPrimary()){
		if(exprPostfixPrim()){
			return true;
		}
	}
	return false;
}
bool exprPostfixPrim(){
	if(consume(LBRACKET)){
		if(expr()){
			if(consume(RBRACKET)){
				if(exprPostfixPrim()){
					return true;
				}
			}else tkerr("syntax error: expected ']' after array index");
		}else tkerr("syntax error: expected expression in array index");
	}
	else if(consume(DOT)){
		if(consume(ID)){
			if(exprPostfixPrim()){
				return true;
			}
		}else tkerr("syntax error: expected field name after '.'");
	}
	return true;
}
/*exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
 	| INT | DOUBLE | CHAR | STRING | LPAR expr RPAR*/
bool exprPrimary(){
	if(consume(ID)){
		if(consume(LPAR)){
			if(expr()){
				while(consume(COMMA)){
					if(!expr()) tkerr("syntax error: expected expression after ','");
				}
			}
			if(!consume(RPAR)) tkerr("syntax error: expected ')'");
		}
		return true;
	}
	if(consume(INT) || consume(DOUBLE) || consume(CHAR) || consume(STRING)){
		return true;
	}
	if(consume(LPAR)){
		if(expr()){
			if(consume(RPAR)){
				return true;
			}else tkerr("syntax error: expected ')'");
		}else tkerr("syntax error: expected expression after '('");
	}
	return false;
}
void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
}
