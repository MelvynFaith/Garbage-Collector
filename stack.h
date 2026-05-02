#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#define STACK_INITIAL_CAPACITY 8

typedef struct {
    void   **items;  
    size_t   count;   
    size_t   capacity;
} Stack;

void  stack_init(Stack *s);

void  stack_free(Stack *s);

void  stack_push(Stack *s, void *item);

void *stack_pop(Stack *s);

void *stack_peek(const Stack *s);

#endif 