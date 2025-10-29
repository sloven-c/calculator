//
// Created by marto on 27. 10. 25.
//

#ifndef CALCULATOR_STACK_H
#define CALCULATOR_STACK_H

typedef enum {
    StringArray,
    CharArray,
    IntArray,
} DataType;

typedef struct {
    DataType type;

    union {
        char **sarr;
        char *carr;
        int *narr;
    } data;

    // if i >= len, the program's guardrails have failed
    int i, len;
} stack;

typedef union {
    char ch;
    char *string;
    int n;
} stackInput;

typedef struct {
    stackInput data;
    int ret_code;
} stackData;

stack stack_init(DataType type, int len);

void stack_deinit(stack *stack);

int stack_push(stack *stack, stackInput input, bool pushCharToString);

stackData stack_pop(stack *stack);

stackData stack_get(const stack *stack, int n);

void stack_print(const stack *stack, const char *name);

static void clear_str_array(const stack *stack);

#endif //CALCULATOR_STACK_H
