#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "structures.h"

int calculate_stack(const stack *output_queue);

int str_to_num(char *string);

int calculate(int a, int b, char op);

string get_expression(int argc, char *argv[]);

stack get_output_queue(const stack *tokenisedExpression);

int push_operator(char operatorToPush, stack *output_stack, stack *operator_stack);

bool stack_higher_precedence(char op, char stackOp);

stack split_into_tokens(string expression);

string get_slice(int a, int b, const char *expression);

bool validate_expression(const char *expression);

charResult validate_char(char c);

void validate_malloc(const void *p);

void throw_error(char *msg);

int main(const int argc, char *argv[]) {
    /*
     * TODO
     * Implement calculating with negative numbers
     * and if you're suicidal enough calculation with floats
     * Fix strlen performance costs
     * Maybe also improve our exception/error handling
     */
    const string expression = get_expression(argc, argv);
    if (expression.string == nullptr) {
        fprintf(stderr, "Couldn't parse the expression\n");
        return 0;
    }

    stack tokens = split_into_tokens(expression);
    // stack_print(&tokens, "tokens");

    stack output_queue = get_output_queue(&tokens);

    // stack_print(&output_queue, "Output");
    printf("%d\n", calculate_stack(&output_queue));

    stack_deinit(&output_queue);
    stack_deinit(&tokens);
    free(expression.string);

    return 0;
}

int calculate_stack(const stack *output_queue) {
    stack calculation_stack = stack_init(IntArray, output_queue->len);

    for (int i = 0; i <= output_queue->i; i++) {
        const stackData element = stack_get(output_queue, i);
        if (element.ret_code == 1)
            throw_error("Couldn't retrieve element from output queue for calculation stack push");

        switch (validate_char(element.data.string[0]).type) {
            case Digit:
                const int n = str_to_num(element.data.string);
                const int r = stack_push(&calculation_stack, (stackInput){.n = n}, false);
                if (r == 1) throw_error("Failed to push number to calculation stack");
                break;
            case Operator:
                const stackData bP = stack_pop(&calculation_stack);
                const stackData aP = stack_pop(&calculation_stack);
                if (aP.ret_code == 1 || bP.ret_code == 1)
                    throw_error(
                        "Failed to pop numbers from calculation stack");

                const int result = calculate(aP.data.n, bP.data.n, element.data.string[0]);
                const int rSp = stack_push(&calculation_stack, (stackInput){.n = result}, false);
                if (rSp == 1) throw_error("Error while trying to push result into calculation stack");

                break;
            case Invalid:
                fprintf(stderr, "The following string is not valid part of expression: '%s'\n",
                        element.data.string);
                throw_error(nullptr);
        }
    }

    const stackData res = stack_get(&calculation_stack, 0);
    if (res.ret_code == 1) {
        fprintf(stderr, "Calculation stack index should be at position 0 but is at %d!\n", calculation_stack.i);
        throw_error(nullptr);
    }

    return res.data.n;
}

int str_to_num(char *string) {
    char *endptr;
    const int num = (int) strtol(string, &endptr, 10);

    if (endptr == string || *endptr != 0x0) {
        fprintf(stderr, "Failed to convert the '%s' string to number\n", string);
        throw_error(nullptr);
    }

    return num;
}

// ReSharper disable once CppNotAllPathsReturnValue
int calculate(const int a, const int b, const char op) {
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (op) { // NOLINT(*-multiway-paths-covered)
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            return a / b;
        case '^':
            return (int) pow(a, b);
    }

    fprintf(stderr, "Operator '%c' is invalid\n", op);
    throw_error(nullptr);
}

string get_expression(const int argc, char *argv[]) {
    string ex = {.string = nullptr};
    if (argc <= 1) return ex; // first argument is just program name

    int len = 0;
    for (size_t i = 1; i < argc; i++) {
        len += (int) strlen(argv[i]);
    }
    const int totalLen = len * (int) sizeof(char) + 1; // +1 => \0 (0x0)

    char *expression = malloc(totalLen);
    validate_malloc(expression);

    expression[0] = '\0'; // since the following will cat string, we need to set the end or rather start point

    for (size_t i = 1; i < argc; i++) {
        strlcat(expression, argv[i], totalLen);
    }

    if (validate_expression(expression)) {
        ex.string = expression;
        ex.len = totalLen;
    } else {
        free(expression);
    }

    return ex;
}

// we could not return anything and just send pointer to stack?
stack get_output_queue(const stack *tokenisedExpression) {
    stack output_stack = stack_init(StringArray, tokenisedExpression->len);
    stack operator_stack = stack_init(CharArray, tokenisedExpression->len);

    for (int i = 0; i <= tokenisedExpression->i; i++) {
        const stackData token = stack_get(tokenisedExpression, i);
        if (token.ret_code == 1) throw_error("Failed to retrieve token");

        if (validate_char(token.data.string[0]).type == Operator) {
            // do operator shit
            const int ret = push_operator(token.data.string[0], &output_stack, &operator_stack);
            if (ret == 1) throw_error("Failed to push operator");
        } else {
            // push number to output queue
            stack_push(&output_stack, token.data, false);
        }

        // printf("pushing output stack, i = %lu\n", i);
        // stack_print(&output_stack, "Output");
    }

    // pop everything from operator to output stack
    while (operator_stack.i >= 0) {
        const stackData popOp = stack_pop(&operator_stack);
        if (popOp.ret_code == 1) throw_error("Failed to retrieve operator from stack");

        const int ret = stack_push(&output_stack, popOp.data, true);
        if (ret == 1) throw_error("Failed to push operator to output");
    }

    stack_deinit(&operator_stack);
    return output_stack;
}

// push operator to operator queue and
int push_operator(const char operatorToPush, stack *output_stack, stack *operator_stack) {
    if (operatorToPush == '(') {
        return stack_push(operator_stack, (stackInput){.ch = operatorToPush}, false);
    }
    if (operatorToPush == ')') {
        for (stackData el = stack_pop(operator_stack); el.data.ch != '('; el = stack_pop(operator_stack)) {
            if (el.ret_code == 1) return 1;

            const int r = stack_push(output_stack, el.data, true);
            if (r == 1) return 1;
        }

        return 0;
    }

    stackData lastStackOp = stack_get(operator_stack, operator_stack->i);
    while (lastStackOp.ret_code == 0 && stack_higher_precedence(operatorToPush, lastStackOp.data.ch)) {
        const stackData opToPush = stack_pop(operator_stack);
        if (opToPush.ret_code == 1) return 1;

        const int r = stack_push(output_stack, opToPush.data, true);
        if (r == 1) return 1;
        lastStackOp = stack_get(operator_stack, operator_stack->i);
    }

    return stack_push(operator_stack, (stackInput){.ch = operatorToPush}, true);
}

bool stack_higher_precedence(const char op, const char stackOp) {
    const int stackOpPrecedence = validate_char(stackOp).opPrecedence, opPrecedence = validate_char(op).
            opPrecedence;
    // -1 precedence is returned only when one of the operator are brackets ()
    if (stackOpPrecedence == -1 || opPrecedence == -1) return false;

    return stackOpPrecedence > opPrecedence;
}

stack split_into_tokens(const string expression) {
    stack tokenisedExpression = stack_init(StringArray, expression.len);
    int lastOp = -1;

    // -1 is to not push \0 onto stack
    for (int i = 0; i < expression.len; i++) {
        // we track if we reached \0 so we can make sure the  last number gets pushed onto the stack
        // as we only do stack pushing when we reach an operator or in this case \0
        const bool reached_eof = expression.string[i] == 0x0 && lastOp != i - 1;
        if (validate_char(expression.string[i]).type != Operator && !reached_eof) continue;

        const string slice = get_slice(lastOp, i, expression.string);
        if (slice.string != nullptr) {
            const int ret = stack_push(&tokenisedExpression, (stackInput){.string = slice.string}, false);
            free(slice.string);
            if (ret == 1) throw_error("Failed to push slice into stack");
        }

        if (reached_eof) break; // do not push \0 into token stack
        const int ret = stack_push(&tokenisedExpression, (stackInput){.ch = expression.string[i]}, true);
        if (ret == 1) throw_error("Failed to push current operator into stack");

        lastOp = i;
    }

    return tokenisedExpression;
}

string get_slice(int a, const int b, const char *expression) {
    const int len = b - a - 1;
    if (len < 1) return (string){.string = nullptr};

    const int total_len = len * (int) sizeof(char) + 1;

    // disregard malloc warnings the caller frees the memory
    char *slice = malloc(total_len);
    validate_malloc(slice);

    for (const size_t i = ++a; a < b; a++) {
        // we don't track i separately but how much has 'a' moved since we started creating the slice
        slice[a - i] = expression[a];
    }
    slice[len] = 0x0;

    return (string){slice, total_len};
}

bool validate_expression(const char *expression) {
    for (const char *it = expression; *it != '\0'; it++) {
        if (validate_char(*it).type == Invalid) return false;
    }

    return true;
}

charResult validate_char(const char c) {
    if (isdigit(c)) return (charResult){Digit, 0};
    // todo maybe we it'd be better to just have it as const instead of creating it every time we call this function
    const char *OPERATORS[] = {"+-", "*/", "^", "()"};
    constexpr int OP_LEN = sizeof(OPERATORS) / sizeof(OPERATORS[0]);

    for (int i = 0; i < OP_LEN; i++) {
        for (int j = 0; j < strlen(OPERATORS[i]); j++) {
            if (c == OPERATORS[i][j])
                // if the operators are parenthesis return 'invalid' precedence
                return (charResult){Operator, i == OP_LEN - 1 ? -1 : i};
        }
    }

    return (charResult){Invalid, 0};
}

void validate_malloc(const void *p) {
    if (p != NULL) return;

    throw_error("Failed to allocate memory");
}

void throw_error(char *msg) {
    if (msg != nullptr)
        fprintf(stderr, "%s\n", msg);

    exit(1);
}
