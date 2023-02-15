#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"



// Create a new NULL_TYPE value node.
Value *makeNull(void) {
    Value *value = talloc(sizeof(Value));
    value->type = NULL_TYPE;
    return value;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr) {
    Value *cons = talloc(sizeof(Value));
    cons->type = CONS_TYPE;
    cons->c.car = newCar;
    cons->c.cdr = newCdr;
 return cons;
}

void displayHelper(Value *list){
    if(list->type != CONS_TYPE){
        switch(list->type){
            case INT_TYPE:
                printf("%i ", list->i);
                break;
            case DOUBLE_TYPE:
                printf("%lf ", list->d);
                break;
            case BOOL_TYPE:
            case STR_TYPE:
                printf("%s ", (list->s));
                break;
            default:
                break;
        }
    }else{
        displayHelper(list->c.car);
        displayHelper(list->c.cdr);
    }
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list) {
    printf("'(");
    displayHelper(list);
    printf(")\n");
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
Value *reverseHelper(Value *list, Value *startList) {
    if(list->type == NULL_TYPE){
        return startList;
    }/*else if(list->type != CONS_TYPE && list->type != NULL_TYPE) {
        Value *val = talloc(sizeof(Value));
        switch(list->type){
            case INT_TYPE:
                val->type = INT_TYPE;
                val->i = list->i;
                break;
            case DOUBLE_TYPE:
                val->type = DOUBLE_TYPE;
                val->d = list->d;
                break;
            case STR_TYPE:
                val->type = STR_TYPE;
                val->s = talloc(sizeof(char)*(strlen(list->s) + 1));
                val->s = list->s;
                break;
            default:
                break;
        }
        return val;
    }*/
    else {  
        startList = cons(list->c.car, startList);
        return reverseHelper(list->c.cdr, startList);
    }
}

Value *reverse(Value *list) {
    Value *startList = makeNull();
    return reverseHelper(list, startList); 
}


// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.

Value *car(Value *list) {
    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list) {
    return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
    if(value->type == NULL_TYPE){
        return true;
    }
    return false;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value) {
    if(isNull(value)) {
        return 0;
    }
    else{
        if(value->type == CONS_TYPE){
            return (length(car(value))+ length(cdr(value)));
        } else {
            return 1;
        }
    }
}


/*
int main()
{
    Value *val1 = talloc(sizeof(Value));
    val1->type = INT_TYPE;
    val1->i = 1;
    Value *val2 = talloc(sizeof(Value));
    val2->type = DOUBLE_TYPE;
    val2->d = 2.0;
    Value *val3 = talloc(sizeof(Value));
    val3->type = STR_TYPE;
    char *text = "We (Nana & Roo) are very much done with all of this bull****";
    val3->s = talloc(sizeof(char)*(strlen(text) + 1));
    strcpy(val3->s,text);
    Value *val4 = makeNull();
    Value *list = cons(val1, cons(val2, cons(val3, val4)));

    display(list);
    Value *reversedList = reverse(list);
    display(reversedList);
    
    printf("%i\n", length(list));
    printf("%i\n", length(reversedList));

    tfree();
    return 0;
}
*/