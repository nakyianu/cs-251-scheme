#include "value.h"
#include "tokenizer.h"
#include "linkedlist.h"
#include "talloc.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

int isSymbol(char charRead){
    if(isalpha(charRead)){
        return 1;
    }
    switch(charRead){
        case '$':
        case '!':
        case '%':
        case '&':
        case '*':
        case '/':
        case ':':
        case '<':
        case '=':
        case '>':
        case '?':
        case '~':
        case '_':
        case '^':
            return 1;
            break;
        default:
            return 0;
            break;

    }
}

//checks to see if charachter is a valid char for being a non-leading char in a symbol/identifier. 
int isInternalSymbol(char charRead){
    if(isSymbol(charRead) || isdigit(charRead) || charRead == '.' || charRead == '+' || charRead == '-')
        return true;
    else
        return false;
}

//creates a Value object for a symbol
Value *symbSeq(charRead) {
    //printf("WE ARE IN SYMBOL\n");
    char *temp = talloc(sizeof(char) * 301);
    temp[0] = charRead;
    int i = 1;
    charRead = (char)fgetc(stdin);
    while(isInternalSymbol(charRead)) {
        temp[i] = charRead;
        i++;
        charRead = (char)fgetc(stdin);
    }
    ungetc(charRead, stdin);
    temp[i] = '\0';
    Value *tempcar = talloc(sizeof(Value));
    tempcar->type = SYMBOL_TYPE;
    tempcar->s = temp;
    return tempcar;
}

// creates a Value object for a string
Value *strSeq(charRead){
    int i = 1;
    char *temp = talloc(sizeof(char)*(303));
    charRead = (char)fgetc(stdin);
    while(charRead != '"') {
        if(charRead == EOF) {
            printf("Syntax error: mismatched quotes\n");
            texit(1);
        }
        temp[i] = charRead;
        i++;
        charRead = (char)fgetc(stdin);            
    }
    temp[0] = '"';
    temp[i] = '"';
    temp[i+1] = '\0';
    Value *tempcar = talloc(sizeof(Value));
    tempcar->type = STR_TYPE;
    tempcar->s = temp;
    return tempcar;
}

// creates a Value object for a an int or double
Value *digiSeq(charRead){
    int i = 0;
    char *temp = talloc(sizeof(char)*(301));
    int decimal = 0;
    while(charRead != ' ' && charRead != EOF && charRead != '\n') {
        if(isalpha(charRead) || isSymbol(charRead)) {
            printf("Syntax error: invalid format for identifier.\n");
            texit(1);
        }
        else if(charRead == ')' || charRead == '('){
            break;
        }else{
            temp[i] = charRead;
            i++;
            charRead = (char)fgetc(stdin);
            if(charRead == '.'){
                decimal++;
            }
        }
    }

    if(charRead != ' '){
        ungetc(charRead, stdin);
    }
    // determines if the number provided is a valid double, int, or invalid (by having more than 1 decimal point)
    temp[i] = '\0';
    char *ptr;
    Value *tempcar = talloc(sizeof(Value));
    if(decimal){
        if(decimal > 1){
            printf("Syntax error: multiple decimal points in double type\n");
            texit(1);
        }else{
            tempcar->type = DOUBLE_TYPE;
            tempcar->d = strtod(temp, &ptr);
        }
    }else{
            tempcar->type = INT_TYPE;
            tempcar->i = strtol(temp, &ptr, 10);
    }
    return tempcar;
}

// creates a Value object for a sign (+ or -)
Value *signSeq(charRead){
    char *temp = talloc(sizeof(char)*302);
    temp[0] = charRead;
    charRead = (char)fgetc(stdin);
    Value *tempcar = talloc(sizeof(Value));
    if(!isdigit(charRead)) {
        ungetc(charRead, stdin);
        temp[1] = '\0';
        tempcar->type = SYMBOL_TYPE;
        tempcar->s = temp;
    }
    //checks if number is associated with the sign. If it is, the program runs digiSeq, and modifies the result if the sign is minus.
    else{
        tempcar = digiSeq(charRead);
        if (temp[0] == '-'){
            if(tempcar->type == DOUBLE_TYPE){
                tempcar->d *= -1;
            }else{
                tempcar->i *= -1;
            }
        }
    }
    return tempcar;
}

// strips out comments
int commSeq(charRead){
    while(charRead != EOF && charRead != '\n') {
        charRead = (char)fgetc(stdin);
    }
    if(charRead == EOF)
        return 1;
    else
        return 0;
}

Value *boolSeq(charRead){
    char *temp = talloc(sizeof(char)*3);
    temp[0] = charRead;
    charRead = (char)fgetc(stdin);
    if(charRead != 't' && charRead != 'f'){
        printf("Syntax error: untokenizable: #%d. Expected #f or #t\n", charRead);
        texit(1);
    }
    temp[1] = charRead; 
    temp[2] = '\0';
    Value *tempcar = talloc(sizeof(Value));
    tempcar->type = BOOL_TYPE;
    tempcar->s = temp;
    return tempcar;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
    while(list->type != NULL_TYPE){
        Value *val = list->c.car;
        switch(val->type){
            case DOUBLE_TYPE:
                printf("%f:double\n", val->d);
                break;
            case OPEN_TYPE:
                printf("(:open\n");
                break;
            case CLOSE_TYPE:
                printf("):close\n");
                break;
            case BOOL_TYPE:
                printf("%s:boolean\n", val->s);
                break;
            case STR_TYPE:   
                printf("%s:string\n", val->s);
                break;
            case SYMBOL_TYPE:
                printf("%s:symbol\n", val->s);
                break;
            case INT_TYPE:
                printf("%i:integer\n", val->i);
                break;            

            default:
                break;
            
        }
        list = list->c.cdr;
    }
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    while (charRead != EOF){
        //skips spaces and newline chars
        if(charRead == ' ' || charRead == '\n'){
            charRead = (char)fgetc(stdin);
            continue;
        }
        //comment check
        else if(charRead == ';') {
            if(commSeq(charRead)){
                break;
            }else{
                charRead = (char)fgetc(stdin);
                ungetc(charRead, stdin);
            }
        }

        //open and close parens
        else if(charRead == '('){
            Value *tempcar = talloc(sizeof(Value));
            tempcar->type = OPEN_TYPE;
            list = cons(tempcar, list);
        }
        else if(charRead == ')'){
            Value *tempcar = talloc(sizeof(Value));
            tempcar->type = CLOSE_TYPE;
            list = cons(tempcar, list);
        }
        //symbol check
        else if(isSymbol(charRead)){
            list = cons(symbSeq(charRead), list);
        }
        //string check
        else if(charRead == '"'){
            list = cons(strSeq(charRead), list);
        }
        //sign check
        else if(charRead == '+'|| charRead == '-') {
            list = cons(signSeq(charRead), list);
        }
        //bool check
        else if(charRead == '#') {
            list = cons(boolSeq(charRead), list);
        }
        //number check
        else if (isdigit(charRead)){
            list = cons(digiSeq(charRead), list);
        }
        //anything else weird that won't hit the above.
        else{
            printf("Syntax error: Unkown token\n");
            texit(1);
        }
        //keep on iterating through the loop, traverses stdin
        //carry on, my wayward charRead...
        charRead = (char)fgetc(stdin);
    }

    //reverses list.
    Value *reversed = reverse(list);
    return reversed;

    // We're done. Thank god.
}
