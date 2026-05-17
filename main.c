#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include "ad.h"
#include "at.h"

int main()
{
    //analizator lexical
    /*char *buffer=loadFile("testlex.c");
    Token *tokens=tokenize(buffer);
    printf("Tokenii generati:\n");
    showTokens(tokens);
    free(buffer);*/

    //analizator sintactic
    /*char *parserBuffer=loadFile("tests/testparser.c");
    Token *parserTokens=tokenize(parserBuffer);
    parse(parserTokens);
    free(parserBuffer);*/
    //analizator de domeniu
    /*
    char *adBuffer=loadFile("tests/testad.c");
    Token *adTokens=tokenize(adBuffer);
    pushDomain();
    parse(adTokens);
    showDomain(symTable, "global");
    dropDomain();
    free(adBuffer);*/
    //analizator de tipuri
    char *atBuffer=loadFile("tests/testat.c");
    Token *atTokens=tokenize(atBuffer);
    pushDomain();
    parse(atTokens);
    showDomain(symTable, "global");
    dropDomain();
    free(atBuffer);
    return 0;
}