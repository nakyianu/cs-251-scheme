#include <stdlib.h>
#include "value.h"
//#include "talloc.h"


Value *head = NULL;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    void *val = malloc(size);

    Value *newhead = malloc(sizeof(Value));
    newhead->type = CONS_TYPE;
    Value *carPointer = malloc(sizeof(Value));
    carPointer->type = PTR_TYPE;
    carPointer->p = val;
    newhead->c.car = carPointer;
    newhead->c.cdr = head;
    head = newhead;

    return val;
}


// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree() {
    while (head != NULL){
        Value *pointer = (head->c.car)->p;
        free(pointer);
        free(head->c.car);
        Value *tempHead = head;
        head = head->c.cdr;
        free(tempHead);
    }
    free (head);
}


// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}



