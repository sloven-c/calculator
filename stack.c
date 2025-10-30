//
// Created by marto on 27. 10. 25.
//

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#include <string.h>

stack stack_init(const DataType type, const int len) {
    stack stack = {
        .type = type,
        .i = -1,
        .len = len,
    };

    const size_t arrLen = len;

    switch (type) {
        case StringArray:
            stack.data.sarr = malloc(arrLen * sizeof(char *));
            break;
        case CharArray:
            stack.data.carr = malloc(arrLen * sizeof(char));
            break;
        case IntArray:
            stack.data.narr = malloc(arrLen * sizeof(int));
        default:
            fprintf(stderr, "Failed to recognise the DataType for Stack structure");
            exit(1);
    }

    // ReSharper disable once CppSomeObjectMembersMightNotBeInitialized
    return stack;
}

void stack_deinit(stack *stack) {
    switch (stack->type) {
        case StringArray:
            clear_str_array(stack);
            free(stack->data.sarr);
            stack->data.sarr = nullptr;
            break;
        case CharArray:
            free(stack->data.carr);
            stack->data.carr = nullptr;
            break;
        case IntArray:
            free(stack->data.narr);
            stack->data.narr = nullptr;
            break;
    }
}

int stack_push(stack *stack, const stackInput input, const bool pushCharToString) {
    if (stack->i >= stack->len - 1) return 1;
    stack->i++;

    switch (stack->type) {
        case StringArray:
            char *str;

            // if we still push to array of strings but we're pushing single character as string
            if (pushCharToString) {
                str = malloc(sizeof(char) + 1);
                str[0] = input.ch;
                str[1] = 0x0;
            } else {
                str = malloc(strlen(input.string) * sizeof(char) + 1);
                strcpy(str, input.string);
            }

            stack->data.sarr[stack->i] = str;
            break;
        case CharArray:
            stack->data.carr[stack->i] = input.ch;
            break;
        case IntArray:
            stack->data.narr[stack->i] = input.n;
    }

    return 0;
}

stackData stack_pop(stack *stack) {
    if (stack->i == -1) return (stackData){.ret_code = 1};
    stackInput returnData;

    switch (stack->type) {
        case StringArray:
            returnData.string = stack->data.sarr[stack->i];
            // todo might have to do malloc?
            // todo ticking timebomb
            break;
        case CharArray:
            returnData.ch = stack->data.carr[stack->i];
            break;
        case IntArray:
            returnData.n = stack->data.narr[stack->i];
            break;
    }

    stack->i--;
    return (stackData){returnData, 0};
}

stackData stack_get(const stack *stack, const int n) {
    if (stack->i == -1 || n > stack->i) return (stackData){.ret_code = 1};
    stackInput returnData;

    switch (stack->type) {
        case StringArray:
            // todo do not forget to think about freeing memory
            const size_t len = strlen(stack->data.sarr[n]);
            returnData.string = malloc(len * sizeof(char) + 1);
            strcpy(returnData.string, stack->data.sarr[n]);
            break;
        case CharArray:
            returnData.ch = stack->data.carr[n];
            break;
        case IntArray:
            returnData.n = stack->data.narr[n];
            break;
    }

    return (stackData){returnData, 0};
}

void stack_print(const stack *stack, const char *name) {
    printf("Printing stack %s:\n", name);
    if (stack->i == -1) return (void) printf("NULL\n");

    for (int i = 0; i <= stack->i; i++) {
        const stackData el = stack_get(stack, i);
        if (el.ret_code == 1) {
            fprintf(stderr, "Invalid index\n");
            exit(1);
        }

        printf("[%d]: ", i);

        switch (stack->type) {
            case StringArray:
                printf("'%s'", el.data.string);
                break;
            case CharArray:
                printf("'%c'", el.data.ch);
                break;
            case IntArray:
                printf("'%d'", el.data.n);
                break;
        }

        printf("\n");
    }
    printf("i location: %d\n", stack->i);
}

static void clear_str_array(const stack *stack) {
    for (size_t i = 0; i < stack->len; i++) {
        free(stack->data.sarr[i]);
        stack->data.sarr[i] = nullptr;
    }
}
