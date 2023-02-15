#ifndef _INTERPRETER
#define _INTERPRETER

Value *lookUpSymbol(Value *symbol, Frame *frame);
Value *evalIf(Value* args, Frame* frame);
Value *defineLambda(Value *args, Frame *frame);
Value *evalLet(Value* args, Frame* frame);
void interpret(Value *tree);
Value *eval(Value *tree, Frame *frame);
int letBindingCheck(Value *binding);
Value *evalQuote(Value * args);
Value *applyHelper(Value *function, Value *args, Frame *frame);
Value *apply(Value *function, Value *args, Frame *frame);
Value *evalDefine(Value *args, Frame *frame);
Value *plus(Value *args);
Value *null(Value *args);
Value *carFn(Value *args);
Value *cdrFn(Value *args);
Value *consFn(Value *args);

#endif

