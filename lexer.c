#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
//FA ERORILE PT CHAR/INT ETc
#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;		// the last token in list

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code){
	Token *tk=safeAlloc(sizeof(Token));
	tk->code=code;
	tk->line=line;
	tk->next=NULL;
	if(lastTk){
		lastTk->next=tk;
		}else{
		tokens=tk;
		}
	lastTk=tk;
	return tk;
	}

char *extract(const char *begin,const char *end){
	int lin=end-begin;
	char *text=safeAlloc((lin+1)*sizeof(char));
	memcpy(text,begin,lin);
	text[lin]='\0';
	return text;
	}

Token *tokenize(const char *pch){
	const char *start;
	Token *tk;
	for(;;){
		switch(*pch){
			case ' ':case '\t':pch++;break;
			case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
				if(pch[1]=='\n')pch++;
				// fallthrough to \n
			case '\n':
				line++;
				pch++;
				break;
			//delimitatori
			case '\0':addTk(END);return tokens;
			case ',':addTk(COMMA);pch++;break;
			case ';':addTk(SEMICOLON);pch++;break;
			case '(':addTk(LPAR);pch++;break;
			case ')':addTk(RPAR);pch++;break;
			case '[':addTk(LBRACKET);pch++;break;
			case ']':addTk(RBRACKET);pch++;break;
			case '{':addTk(LACC);pch++;break;
			case '}':addTk(RACC);pch++;break;
			//operatori
			case '+':addTk(ADD);pch++;break;
			case '-':addTk(SUB);pch++;break;
			case '*':addTk(MUL);pch++;break;
			case '.':addTk(DOT);pch++;break;
			case '&':
				if(pch[1]=='&'){
					addTk(AND);
					pch+=2;
					}else{
					err("invalid char: &");
					pch++;
					}
				break;
			case '|':
				if(pch[1]=='|'){
					addTk(OR);
					pch+=2;
					}else{
					err("invalid char: |");
					pch++;
					}
				break;
			case '=':
				if(pch[1]=='='){
					addTk(EQUAL);
					pch+=2;
					}else{
					addTk(ASSIGN);
					pch++;
					}
				break;
			case '!':
				if(pch[1]=='='){
					addTk(NOTEQ);
					pch+=2;
					}else{
						addTk(NOT);
						pch++;
					}
				break;
			case '<':
				if(pch[1]=='='){
					addTk(LESSEQ);
					pch+=2;
					}else{
					addTk(LESS);
					pch++;
					}
				break;
			case '>':
				if(pch[1]=='='){
					addTk(GREATEREQ);
					pch+=2;
					}else{
					addTk(GREATER);
					pch++;
					}
				break;
			case '/':
				if(pch[1]=='/'){
					pch+=2;
					while(*pch!='\0' && *pch!='\n' && *pch!='\r') pch++;
				}else{
					addTk(DIV);
					pch++;
				}	
				break;		
			case '\'':
				if(pch[1]=='\'') err("null char constant");
				char c;
				if(pch[1]=='\\'){
					switch(pch[2]){
						case 'a':c='\a'; break;
						case 'b':c='\b'; break;
						case 'f':c='\f'; break;
						case 'n':c='\n'; break;
						case 'r':c='\r'; break;
						case 't':c='\t'; break;
						case 'v':c='\v'; break;
						case '\\':c='\\'; break;
						case '\'':c='\''; break;
						case '\"':c='\"'; break;
						case '0':c='\0'; break;
						default: err("invalid escape sequence in char constant");
					}
					if(pch[3]!='\'') err("missing ' at the end of char constant");
					tk=addTk(CHAR);
					tk->c=c;
					pch+=4;
				}
				else{
					if(pch[2] != '\'') err("missing ' at the end of char constant");
					c=pch[1];
					tk=addTk(CHAR);
					tk->c=c;
					pch+=3;
				}
				break;
			case '\"':
				char *text=safeAlloc(strlen(pch)+1);
				int i=0;
				char x;
				pch++;	// jump over "
				for(;;){
					if(*pch=='\0') err("missing \" at the end of the string");
					if(*pch=='"'){
						text[i]='\0';
						tk=addTk(STRING);
						tk->text=text;
						pch++;
						break;
					}
					if(*pch == '\\'){
						pch++;
						switch(*pch) {
							case 'a':x='\a'; break;
							case 'b':x='\b'; break;
							case 'f':x='\f'; break;
							case 'n':x='\n'; break;
							case 'r':x='\r'; break;
							case 't':x='\t'; break;
							case 'v':x='\v'; break;
							case '\\':x='\\'; break;
							case '\'':x='\''; break;
							case '\"':x='\"'; break;
							case '0':x='\0'; break;
							default: err("invalid escape sequence in string");
						}
						text[i]=x;
						i++;
						pch++;
					}
					else{
						text[i]=*pch;
						i++;
						pch++;
					}
				}
			break;
			default:
				if(isalpha(*pch)||*pch=='_'){
					for(start=pch++;isalnum(*pch)||*pch=='_';pch++){}
					char *text=extract(start,pch);
					//cuvinte cheie
					if(strcmp(text,"char")==0)addTk(TYPE_CHAR);
					else if(strcmp(text,"double")==0)addTk(TYPE_DOUBLE);
					else if(strcmp(text,"int")==0)addTk(TYPE_INT);
					else if(strcmp(text,"if")==0)addTk(IF);
					else if(strcmp(text,"else")==0)addTk(ELSE);
					else if(strcmp(text,"while")==0)addTk(WHILE);
					else if(strcmp(text,"return")==0)addTk(RETURN);
					else if(strcmp(text,"void")==0)addTk(VOID);
					else if(strcmp(text,"struct")==0)addTk(STRUCT);
					else {
						tk=addTk(ID);
						tk->text=text;
					}
				}
				else if(isdigit(*pch)){
					start=pch;
					for(pch++;isdigit(*pch);pch++){}
					int isDouble=0;

					if(*pch=='.'){
						if(isdigit(pch[1])){
							isDouble=1;
							pch++;
							while(isdigit(*pch)) pch++;
						}
						else err("at least one digit required after decimal point");
					}

					if(*pch=='e'||*pch=='E'){
						isDouble=1;
						pch++;
						if(*pch=='+'||*pch=='-') pch++;
						if(isdigit(*pch)){
							while(isdigit(*pch)) pch++;
						}
						else err("at least one digit required in exponent");
					}

					char *text=extract(start, pch);

					if(isDouble){
						tk=addTk(DOUBLE);
						tk->d=atof(text);
					}
					else{
						tk=addTk(INT);
						tk->i=atoi(text);
					}
				}
		else err("invalid char: %c (%d)",*pch,*pch);
		}
	}
}
void showTokens(const Token *tokens){
	for(const Token *tk=tokens;tk;tk=tk->next){
		printf("%d\t",tk->line); //se afiseaza linia pt fiecare
		switch(tk->code){
			//keywords
			case ID:printf("ID:%s\n",tk->text);break;
			case INT:printf("INT:%d\n",tk->i);break;
			case DOUBLE:printf("DOUBLE:%.2f\n",tk->d);break;
			case CHAR:printf("CHAR:%c\n",tk->c);break;
			case STRING:printf("STRING:%s\n",tk->text);break;
			case TYPE_CHAR:printf("TYPE_CHAR\n");break;
			case TYPE_DOUBLE:printf("TYPE_DOUBLE\n");break;
			case TYPE_INT:printf("TYPE_INT\n");break;
			case IF:printf("IF\n");break;
			case ELSE:printf("ELSE\n");break;
			case WHILE:printf("WHILE\n");break;
			case RETURN:printf("RETURN\n");break;
			case VOID:printf("VOID\n");break;
			case STRUCT:printf("STRUCT\n");break;
			//delimiters
			case COMMA:printf("COMMA\n");break;
			case SEMICOLON:printf("SEMICOLON\n");break;
			case LPAR:printf("LPAR\n");break;
			case RPAR:printf("RPAR\n");break;
			case LBRACKET:printf("LBRACKET\n");break;
			case RBRACKET:printf("RBRACKET\n");break;
			case LACC:printf("LACC\n");break;
			case RACC:printf("RACC\n");break;
			case END:printf("END\n");break;
			//operators
			case ADD:printf("ADD\n");break;
			case SUB:printf("SUB\n");break;
			case MUL:printf("MUL\n");break;
			case DIV:printf("DIV\n");break;
			case DOT:printf("DOT\n");break;
			case AND:printf("AND\n");break;
			case OR:printf("OR\n");break;
			case NOT:printf("NOT\n");break;
			case ASSIGN:printf("ASSIGN\n");break;
			case EQUAL:printf("EQUAL\n");break;
			case NOTEQ:printf("NOTEQ\n");break;
			case LESS:printf("LESS\n");break;
			case LESSEQ:printf("LESSEQ\n");break;
			case GREATER:printf("GREATER\n");break;
			case GREATEREQ:printf("GREATEREQ\n");break;
		}
	}
}
