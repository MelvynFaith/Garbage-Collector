#ifndef FRAME_H
#define FRAME_H

#include "stack.h"
#include "snake_object.h"


typedef struct {
    Stack references; 
} Frame;

Frame *frame_new(void);

void frame_free(Frame *frame);

void          frame_push_ref(Frame *frame, snake_object *obj);
snake_object *frame_pop_ref(Frame *frame);

#endif