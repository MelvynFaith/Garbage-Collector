#include "stack.h"

#include <stdio.h>   
#include <stdlib.h> 

void stack_init(Stack *s) {
    s->items    = malloc(STACK_INITIAL_CAPACITY * sizeof(void *));
    if (!s->items) {
        fprintf(stderr, "[Stack] Fatal: initial malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    s->count    = 0;
    s->capacity = STACK_INITIAL_CAPACITY;
}

void stack_free(Stack *s) {
    free(s->items);
    s->items    = NULL;
    s->count    = 0;
    s->capacity = 0;
}

void stack_push(Stack *s, void *item) {
    if (s->count == s->capacity) {
        s->capacity *= 2;
        s->items = realloc(s->items, s->capacity * sizeof(void *));
        if (!s->items) {
            fprintf(stderr, "[Stack] Fatal: realloc failed (capacity=%zu).\n",
                    s->capacity);
            exit(EXIT_FAILURE);
        }
    }
    s->items[s->count++] = item;
}

void *stack_pop(Stack *s) {
    if (s->count == 0) {
        return NULL; 
    }
    return s->items[--s->count];
}

void *stack_peek(const Stack *s) {
    if (s->count == 0) {
        return NULL;
    }
    return s->items[s->count - 1];
}