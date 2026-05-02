#include "frame.h"

#include <stdlib.h> 
#include <stdio.h>  

Frame *frame_new(void) {
    Frame *frame = malloc(sizeof(Frame));
    if (!frame) {
        fprintf(stderr, "[Frame] Fatal: malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    stack_init(&frame->references);
    return frame;
}

void frame_free(Frame *frame) {
    if (!frame) return;
    stack_free(&frame->references);
    free(frame);
}

void frame_push_ref(Frame *frame, snake_object *obj) {
    stack_push(&frame->references, obj);
}

snake_object *frame_pop_ref(Frame *frame) {
    return (snake_object *)stack_pop(&frame->references);
}