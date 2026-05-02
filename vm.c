#include "vm.h"

#include <stdlib.h> 
#include <stdio.h> 

VM *vm_new(void) {
    VM *vm = malloc(sizeof(VM));
    if (!vm) {
        fprintf(stderr, "[VM] Fatal: malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    stack_init(&vm->frames);
    stack_init(&vm->objects);
    return vm;
}

void vm_free(VM *vm) {
    if (!vm) return;

    Frame *frame;
    while ((frame = (Frame *)stack_pop(&vm->frames)) != NULL) {
        frame_free(frame);
    }

    stack_free(&vm->frames);
    stack_free(&vm->objects);
    free(vm);
}

void vm_push_frame(VM *vm, Frame *frame) {
    stack_push(&vm->frames, frame);
}

Frame *vm_pop_frame(VM *vm) {
    return (Frame *)stack_pop(&vm->frames);
}

Frame *vm_current_frame(VM *vm) {
    return (Frame *)stack_peek(&vm->frames);
}

void vm_track_object(VM *vm, snake_object *obj) {
    stack_push(&vm->objects, obj);
}