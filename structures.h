//
// Created by marto on 25. 10. 25.
//

#ifndef CALCULATOR_STRUCTURES_H
#define CALCULATOR_STRUCTURES_H

typedef enum {
    Digit,
    Operator,
    Invalid,
} charType;

typedef struct {
    charType type;
    int opPrecedence;
} charResult;

typedef struct {
    char *string;
    int len;
} string;

#endif //CALCULATOR_STRUCTURES_H
