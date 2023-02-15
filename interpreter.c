#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "talloc.h"
#include "value.h"
#include "parser.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "interpreter.h"

void bind(char *name, Value*(*function)(Value *), Frame *frame){
    Value *prim = talloc(sizeof(Value));
    
    prim->type = PRIMITIVE_TYPE;
    prim->pf = function;

    Value *primBinding = makeNull(); 
    Value *nameVal = talloc(sizeof(Value));
    nameVal->type = SYMBOL_TYPE;
    nameVal->s = name;
    primBinding = cons(nameVal, prim);
    frame->bindings = cons(primBinding, frame->bindings);
}

Value *plus(Value *args){
    Value *argsHead = args;
    double returnVal = 0;
    int isINT = 1;
    while(args->type != NULL_TYPE){ 
        if (args->c.car->type == DOUBLE_TYPE){
            isINT = 0;
            returnVal += args->c.car->d;
        }else{
          returnVal += args->c.car->i;
        }
        args = args->c.car;
    }

    Value *result = talloc(sizeof(Value));
    if(isINT){
        result->type = INT_TYPE;
        result->i = (int) returnVal;
    }else{
        result->type = DOUBLE_TYPE;
        result->d = returnVal;
    }
    return result;
}

Value *nullFn(Value *args){
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    if(args != NULL || args->type == NULL_TYPE){
        printf("Evaluation error: Nothing provided to 'null?'\n");
        texit(1);
    }else if (args->c.cdr->type != NULL_TYPE){
        printf("Evaluation error: Too many arguments provided to 'null?'\n");
        texit(1);
    }
    if(isNull(args->c.car)){
        result->s = "#t";
    }else{
        result->s = "#f";
    }
    return result;
}

Value *carFn(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to #<procedure car (_)>\n");
        texit(1);
     } else if(args->c.cdr->type != NULL_TYPE){
        printf("Evaluation error: Too many arguments provided to #<procedure car (_)>\n");
        texit(1);
    } else if(args->c.car->type == NULL_TYPE && args->c.car->type != CONS_TYPE){
         printf("Evaluation error: Wrong type argument in position 1 (expected pair).\n");
        texit(1);
    }
    return car(args->c.car);
    
}

Value *cdrFn(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to #<procedure cdr (_)>\n");
        texit(1);
    } else if(args->c.cdr->type != NULL_TYPE){
        printf("Evaluation error: Too many arguments provided to #<procedure cdr (_)>\n");
        texit(1);
    } else if(args->c.car->type == NULL_TYPE && args->c.car->type != CONS_TYPE){
        printf("Evaluation error: Wrong type argument in position 1 (expected pair).\n");
        texit(1);
    } 
    return cdr(args->c.car);
    
}

Value *consFn(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to #<procedure cdr (_)>\n");
        texit(1);
    }if(args->c.cdr->type == NULL_TYPE){
        printf("Evaluation error: wrong number of arguments to #<procedure cdr (_)>\n");
        texit(1);
    }
    return cons(args->c.car, args->c.cdr->c.car);
}

Value *lookUpSymbol(Value *symbol, Frame *frame){
    Value *ptr = frame->bindings;
    while(ptr->type != NULL_TYPE){      // goes through list of bindings
        if(!strcmp(symbol->s, ptr->c.car->c.car->s)){
            return ptr->c.car->c.cdr;           // returns value if binding exists.
        }
        ptr = ptr->c.cdr;       // traverses list of bindings
    }

    if(frame->parent == NULL){              // checks to see if this is the global frame, returns an empty list
       return makeNull();
    }

    return lookUpSymbol(symbol, frame->parent);         // recursively calls lookUpSymbol on previous frame to find binding.
}

Value *eval(Value *tree, Frame *frame){
   Value *result = makeNull();
   if(tree->type != NULL_TYPE){         // checks to make sure that a NULL_TYPE is not evaluated
        switch (tree->type){         // makes sure that numbers, strings and bools evaluate to themselves.
            case BOOL_TYPE:
            case STR_TYPE:
            case DOUBLE_TYPE:
            case INT_TYPE: {
                return tree;
                break;
            }

            case SYMBOL_TYPE: {
                // Value *first = tree;
                // Value *args = cdr(tree);

                result = lookUpSymbol(tree, frame);
                                
                if(result->type == NULL_TYPE){                  // if lookUpSymbol does not return a value then it throws an error
                    // if(!strcmp(tree->s, "+") || !strcmp(tree->s, "cons") || !strcmp(tree->s, "car") || !strcmp(tree->s, "null?") ||!strcmp(tree->s, "cdr")) {
                    //     return tree;         // makes a check to see if the symbol is a sign, if it is then it returns it as is.
                    // }
                    // else 
                    if(!strcmp(tree->s, "-") || !strcmp(tree->s, "=")){
                        return tree;         // makes a check to see if the symbol is a sign, if it is then it returns it as is.
                    }
                    else {      // throws an error if the symbol is not found in any frame.
                        printf("Evaluation error: symbol '%s' not found.\n", tree->s);
                        texit(1);
                    }
                }
                return result;
                break;
            }
            case CONS_TYPE: {
                printf("in cons case\n");
                Value *first = car(tree);
                Value *args = cdr(tree);

                 if (!strcmp(first->s,"if")){        // runs if evaluation sequence.
                    result = evalIf(args, frame);
                }
                else if(!strcmp(first->s, "let")){      // runs let evaluation sequence.
                    result = evalLet(args, frame);
                }
                else if (!strcmp(first->s,"quote")){        // runs quote evaluation sequence.
                    result = evalQuote(args);
                }
                else if (!strcmp(first->s, "define")){
                    result = evalDefine(args, frame);
                }
                else if(!strcmp(first->s, "lambda")){
                    result = defineLambda(args, frame);
                    
                }
                else if(first->type == CONS_TYPE){
                    return apply(eval(first, frame), args, frame);
                }
                else if(first->type == SYMBOL_TYPE){
                    result = eval(first, frame);
                    printf("in symbol type of cons case\n");
                    printf("%d: type\n", result->type);
                    if (result->type == CLOSURE_TYPE){
                        printf("in closure type\n");
                        result = apply(result, args, frame);
                    }
                    else if(result->type == PRIMITIVE_TYPE){
                        printf("in primitive type \n");
                        return (result->pf)(args);
                    }
                    return result;
                }
                
                else {                  // checks for special forms: if it's not a let or an if or a quote then it is an unknown function.
                    printf("%d\n", first->type);
                    printf("Evaluation error: unrecognized variable\n");
                    texit(1);
                }
               // }
                break;
            }

            default:
                break;
            }
        return result;
    }
    else {
        printf("Evaluation error. Val: %d\n", tree->type);
        texit(1);
    }
    return result;          // returns the result of the evaluation.
}
Value *applyHelper(Value *function, Value *args, Frame *frame){
    Value *result = makeNull();
    Value *binding = makeNull();
    Value *paramPointer = function->cl.paramNames;
    if(length(args) == length(function->cl.paramNames)){
        while(args->type != NULL_TYPE && paramPointer->type != NULL_TYPE){
            binding = cons(paramPointer->c.car, args->c.car);
            frame->bindings = cons(binding, frame->bindings);
            args = args->c.cdr;
            paramPointer = paramPointer->c.cdr;
        }
        return eval(function->cl.functionCode, function->cl.frame);
    } 
    else if(length(args) > length(function->cl.paramNames)){
        printf("Evaluation error: too many arguments passed into function.\n");
        texit(1);
    }
    else{
        printf("Evaluation error: expected more arguments, none found.\n");
        texit(1);
    }
    return result;
}

Value *apply(Value *function, Value *args, Frame *frame){
    if(function->type != CLOSURE_TYPE){
        printf("Evaluation error: wrong typpe to apply.\n");
        texit(1);
    }
    return applyHelper(function, args, function->cl.frame);
    
}

Value *evalDefine(Value *args, Frame *frame){    
    while(frame->parent != NULL){
        frame = frame->parent;
    }
    if (length(args) < 2){
        printf("Evaluation error: no arguments in Define\n");
        texit(1);
    }
    if(args->c.car->type == SYMBOL_TYPE){
        Value *result = talloc(sizeof(Value));
        Value *evaluatedArgs = eval(args->c.cdr->c.car, frame);
        Value *binding = cons(args->c.car, evaluatedArgs);
        frame->bindings = cons(binding, frame->bindings);
        result->type = VOID_TYPE;
        return result;
    }
    else{
        printf("Evaluation error: define parameter does not evaluate to symbol.\n");
        texit(1);
    }
    return makeNull();

}

int identifierCheck(Value *paramList){
    printf("%d: paramlist type\n", paramList->type);
    if(paramList->type == CONS_TYPE){
        if(paramList->c.car->type == CONS_TYPE){
        return 1;
        }
    }
    Value *paramPtr = paramList;
    while(paramPtr->type != NULL_TYPE){
        Value *paramListTraverser = paramPtr->c.cdr;
        while(paramListTraverser->type != NULL_TYPE){
            if(!strcmp(paramPtr->c.car->s, paramListTraverser->c.car->s)){
                printf("Evaluation error: duplicate identifier in lambda.\n");
                texit(1);
            }
            paramListTraverser = paramListTraverser->c.cdr;
        }
        paramList = paramList->c.cdr;
    }
    return 1;
}

Value *defineLambda(Value *args, Frame *frame){
    Value *result = talloc(sizeof(Value));
    if(args->type == CONS_TYPE){
        if(args->c.cdr->type == CONS_TYPE){
            if(args->c.cdr->c.cdr->type == NULL_TYPE){
                result->type = CLOSURE_TYPE;
                Frame *newFrame = talloc(sizeof(Frame));
                newFrame->bindings = makeNull();
                newFrame->parent = frame;
                result->cl.frame = newFrame;
                result->cl.functionCode = args->c.cdr->c.car;
                if(args->c.car->type == NULL_TYPE){
                    result->cl.paramNames = args->c.car;
                }
                else if(args->c.car->type == CONS_TYPE){
                    if(args->c.car->c.car->type == SYMBOL_TYPE){
                    Value *paramsHeader = args->c.car;
                    while (paramsHeader->type != NULL_TYPE){
                        Value *paramsIterator = paramsHeader->c.cdr;
                        while(paramsIterator->type != NULL_TYPE){
                            if (!strcmp(paramsHeader->c.car->s, paramsIterator->c.car->s)){
                                printf("Evaluation error: duplicate identifier in lambda.\n");
                                texit(1);
                            }
                            paramsIterator = paramsIterator->c.cdr;
                        }
                        paramsHeader = paramsHeader->c.cdr;
                    }
                        result->cl.paramNames = args->c.car;
                    } else {
                        printf("Evaluation error: formal parameters for lambda must be symbols.\n");
                        texit(1);
                    }
                } 
            } else{
                printf("Evaluation error: too many arguments passed into lambda.\n");
                texit(1);
            }
        } else {
            printf("Evaluation error: no code in lambda following parameters.\n");
            texit(1);
        }
    } else{
        printf("Evaluation error: no args following lambda.\n");
        texit(1);
    }
    return result;
}   

Value *evalIf(Value *args, Frame* frame){
    Value *result = makeNull();

    if(args->type != NULL_TYPE){        // checks to make sure the right number of arguments are passed into the if statement.
        if(args->c.cdr->type == CONS_TYPE){
            if(args->c.cdr->c.cdr->type == CONS_TYPE){
                if(args->c.cdr->c.cdr->c.cdr->type == NULL_TYPE){
                    if(args->c.car->type == BOOL_TYPE){         // tests to see if the test expression is already a boolean
                        if(!strcmp(args->c.car->s, "#t")){
                            return eval(args->c.cdr->c.car, frame);
                            
                        }
                        else {
                            return eval(args->c.cdr->c.cdr->c.car, frame);
                        }
                    }
                    else {          // if it is not a boolean, it evaluates it and then checks
                        Value *evalResults = eval(args, frame);
                        if(evalResults->type == BOOL_TYPE){         //  checks to make sure the test evaluated to a boolean.
                            if(!strcmp(evalResults->s, "#t")){      // evaluates the consequent if the test evaluates to true.
                                return eval(args->c.cdr->c.car, frame);
                            }
                            else {              // evaluates the alternative otherwise.
                                return eval(args->c.cdr->c.cdr->c.car, frame);
                            }
                        }
                    }
                }
                else {      // throws an error if more than 3 arguments follow the if.
                    printf("Evaluation error: too many arguments passed into if.\n");
                    texit(1);
                }
            }
            else {        // throws an error if only 2 arguments follows the if.
                printf("Evaluation error: no alternative following an if.\n");
                texit(1);
            }
        }
        else {             // throws an error if only 1 arguments follows the if.
            printf("Evaluation error: no consequent following an if.\n");
            texit(1);
        }
        return result;
    }
    else {              // throws an error if no arguments follow the if.
        printf("Evaluation error: no args passed into if.\n");
        texit(1);
    }
    return result;  // returns the result of evaluting the expression.
}

int letBindingCheck(Value *binding){
    if(binding->type != NULL_TYPE && binding->type == CONS_TYPE){   // double checks to make sure the binding points to a list of elements
        if(binding->c.car->type != NULL_TYPE && binding->c.car->type == CONS_TYPE){ // checks to make sure the binding contains a value
            if(binding->c.car->c.car->type == SYMBOL_TYPE){     // checks to make sure that only symbols are bound
                if(binding->c.car->c.cdr->type != NULL_TYPE){   // checks to make sure there's a value for the symbol to bind to.
                    return 1;
                }
                else {      // throws an error if the let contains a value or a symbol without something to bind it to
                    printf("Evaluation error: bad form in let.\n");
                    texit(1);
                }
            }
            else {        // throws an error if the wrong type is being bound to, i.e an integer.
                printf("Evaluation error.\n");
                texit(1);
            }
        }
        else {     // throws an error if the binding is null
            printf("Evaluation error: null binding in let.\n");
            texit(1);
        }
    }
    else {      // throws an error if it was passed in null (probably won't because we make a check for this ealrier but it's just a precaution.)
        printf("Evaluation error: bad form in let.\n");
        texit(1);
    }
        return 0;       // returns 0, because the function needs to return something.
}

Value *evalLet(Value* args, Frame* frame){
    Value *result = makeNull();
    if(args->c.car->type != NULL_TYPE && args->c.cdr->type != NULL_TYPE){    // checks to make sure the let is in the correct form.
        Value *lexpressions = args->c.car;
        Frame *newFrame = talloc(sizeof(Frame));        // creates new frame for let expression
        newFrame->parent = frame;          // assigns parent frame to the frame that was passed in./
        newFrame->bindings = makeNull();
        while(lexpressions->type != NULL_TYPE && letBindingCheck(lexpressions)){     // checks to make sure let expression is in the correct form
            Value *ptr = newFrame->bindings;        // pointer to first item in bindings list.
            while(ptr->type != NULL_TYPE){
                if(!strcmp(lexpressions->c.car->c.car->s, ptr->c.car->c.car->s)){       // if a binding already exists in the current frame, it throws a duplicate binding error.
                    printf("Evaluation error: duplicate variable in let\n");
                    texit(1);
                }
            ptr = ptr->c.cdr;  // updates pointer
            }
                Value *newBinding = cons(lexpressions->c.car->c.car, eval(lexpressions->c.car->c.cdr->c.car, frame));  // if binding is valid, it creates a new binding within the new frame.
                newFrame->bindings = cons(newBinding, newFrame->bindings);
                lexpressions = lexpressions->c.cdr;
        }
        while(args->c.cdr->type != NULL_TYPE){          // evalutes all items in the body but only returns the last one.
            result = eval(args->c.cdr->c.car, newFrame);
            args = args->c.cdr;
        }
    }
    else {
        if(args->c.cdr->type == NULL_TYPE){     // throws an error if the let expression does not have a body.
            printf("Evaluation error: no args following the bindings in let.\n");
            texit(1);
        }
        return eval(args->c.cdr->c.car, frame);        // if let contains no bindings, it evaluates the body, if there is one.
    }
    return result;
}

Value *evalQuote(Value * args){
    if(args->type == NULL_TYPE){                // no args in quote
        printf("Evaluation error - quote nulltype mismatch\n");
        texit(1);
    }
    else if(args->c.cdr->type != NULL_TYPE){                // checks to make sure only one argument is passed in
        printf("Evaluation error: multiple arguments to quote\n");
        texit(1);
    }

    Value *result = args->c.car;                // returns the argument as is
    return result;
}


void interpret(Value *tree){

    Frame *frame = talloc(sizeof(Frame));               // creates global frame.
    frame->parent = NULL;
    frame->bindings = makeNull();
    Value *result = makeNull();
    bind("+", &plus, frame);
    bind("car", &carFn, frame);
    bind("cdr", &cdrFn, frame);
    bind("cons", &consFn, frame);
    bind("null?", &nullFn, frame);
    

    while(tree->type != NULL_TYPE){                  //traverses the tree
        result = cons(eval(tree->c.car, frame), makeNull());
        if(result->type != VOID_TYPE){
            printTree(result);
            printf("\n");
        }
        tree = tree->c.cdr;
        
    }

}
