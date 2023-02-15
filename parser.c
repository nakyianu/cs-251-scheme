#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "tokenizer.h"
#include <stdio.h>


// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){
    Value *list = makeNull();
    int numUnmatchedOpenParens = 0;
    while(tokens->type != NULL_TYPE){
        Value *val = tokens->c.car;
        Value *temp;
        if(val->type == CLOSE_TYPE){
            numUnmatchedOpenParens--;
            Value *encapsulated = makeNull();
            while(list->type != NULL_TYPE && list->c.car->type != OPEN_TYPE){
                encapsulated = cons(list->c.car, encapsulated);
                list = list->c.cdr;
            }
            if(list->type == NULL_TYPE){
                printf("Syntax error: too many close parentheses.\n");
                texit(1);
            }else if(list->c.car->type == OPEN_TYPE){
                reverse(encapsulated);
                list = list->c.cdr;
            }else{
                printf("Error occured. Please check encapsulate code\n");
                texit(1);
            }
            list = cons(encapsulated, list);

        }
        else if(val->type == OPEN_TYPE){
            numUnmatchedOpenParens++;
            list = cons(val, list);
            //list = cons(cons(val, list), makeNull());

        }
        else{
            list = cons(val, list);
        }
        tokens = tokens->c.cdr;
    }

    if(numUnmatchedOpenParens > 0){
        printf("Syntax error: not enough close parenthases\n");
        texit(1);
    }else if(numUnmatchedOpenParens < 0){
        printf("Syntax error: not enough open parenthases\n");
        texit(1);
    }

    list = reverse(list);
    return list;

}

void printHelper(Value *val){
    switch(val->type){
        case CLOSURE_TYPE:
            printf("#<procedure> ");
            break;
        case DOUBLE_TYPE:
            printf("%f ", val->d);
            break;
        case CONS_TYPE:
            printSubTree(val);
            printf(" ");
            break;
        case BOOL_TYPE:
            printf("%s ", val->s);
            break;
        case STR_TYPE:   
            printf("%s ", val->s);
            break;
        case SYMBOL_TYPE:
            printf("%s ", val->s);
            break;
        case INT_TYPE:
            printf("%i ", val->i);
            break;            
        case NULL_TYPE:
            printf("() ");
            break;
        default:
            break;      
    }
}

// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printSubTree(Value *tree){
    printf("(");
    while(tree->type != NULL_TYPE){
    Value *val = tree->c.car;
    printHelper(val);
    tree = tree->c.cdr;
    }
    printf(")");
}

void printTree(Value *tree){
    while(tree->type != NULL_TYPE){
    Value *val = tree->c.car;
    printHelper(val);
    tree = tree->c.cdr;
    }
    // printf("\n");
}


// #include <stdlib.h>
// #include <string.h>
// #include <stdio.h>
// #include "value.h"
// #include "talloc.h"
// #include "linkedlist.h"
// #include "tokenizer.h"


// void printTreeHelper(Value *tree){
//     if(tree->type == NULL_TYPE){ //check for empty parenthesis case.
//         printf("()");
//     }
//     while(tree->type != NULL_TYPE){         //traverses the sublist.
//         Value *val = tree->c.car;
//         switch (val->type){
//             case CONS_TYPE:
//                 printf("( ");
//                 printTreeHelper(val);   //recursively traverses the parse tree
//                 printf(") ");
//                 break;
//             case DOUBLE_TYPE:
//                 printf("%.2f ", val->d);      //formats the double to two decimal places.
//                 break;
//             case INT_TYPE:
//                 printf("%i ", val->i);
//                 break;
//             case STR_TYPE:
//                 printf("%s ", val->s);
//                 break;
//             case SINGLEQUOTE_TYPE:
//                 printf("'");
//                 break;
//             case SYMBOL_TYPE:
//                 printf("%s ", val->s);
//                 break;
//             case BOOL_TYPE:
//                 printf("%s ", val->s);
//                 break;
//         default:
//             break;
//         }
//         tree = tree->c.cdr;        
//     }
//     // printf("\n");

// }


// // Takes a list of tokens from a Scheme program, and returns a pointer to a
// // parse tree representing that program.
// Value *parse(Value *tokens){
//     Value *stack = makeNull();      
//     Value *parseTree = makeNull();
//     int count = 0;

//     while(tokens->type != NULL_TYPE){
//         while(tokens->type != NULL_TYPE && tokens->c.car->type != CLOSE_TYPE){
//             if(tokens->c.car->type == OPEN_TYPE){
//                 count += 1;  //keeps track of the number of open parentheses for parenthesis matching.
//             }
//             stack = cons(tokens->c.car, stack);
//             tokens = tokens->c.cdr;
//         }
//         if(tokens->type == NULL_TYPE && count == 0){        // check to make sure that it doens't call an error for statements without parens
//             Value *temp = makeNull();
//             while(stack->type != NULL_TYPE){        //creates a list of the items
//                 temp = cons(stack->c.car, temp); 
//                 stack = stack->c.cdr;
//             }
//             parseTree = temp;  //returns parse tree.
//             return parseTree;

//         } else if(tokens->type == NULL_TYPE && count > 0){          //check for mismatched parentheses.
//             printf("Syntax error: not enough close parentheses.\n");
//             texit(1);
//         }

//         else {
//             Value *temp = makeNull();
//             while(stack->type != NULL_TYPE && stack->c.car->type != OPEN_TYPE){     
//                 temp = cons(stack->c.car, temp);
//                 stack = stack->c.cdr;
//             }
//             if(stack->type == NULL_TYPE){
//                 printf("Syntax error: too many close parentheses\n");
//                 texit(1);
//             }
//             else{
//                 stack = stack->c.cdr;
//                 stack = cons(temp, stack);
//                 count -= 1;         //decreases number of open parentheses everytime a match is made.
//                 }
//             }
//         tokens = tokens->c.cdr;
//     }
//     if(count > 0){
//         printf("Syntax error: too many open parentheses\n"); 
//         texit(1);
//     } 
//     parseTree = reverse(stack);     //reverses stack to put items in order.
//     return parseTree;
// }
    

// // Prints the tree to the screen in a readable fashion. It should look just like
// // Scheme code; use parentheses to indicate subtrees.
// void printTree(Value *tree){
//         printTreeHelper(tree);  
//         printf("\n");
//     return;
// }

