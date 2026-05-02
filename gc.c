#include "gc.h"

#include <stdio.h>   
#include <stdlib.h>  

void rc_inc(snake_object *obj) {
    if (!obj) return; 
    obj->ref_count++;
}

void rc_dec(snake_object *obj) {
    if (!obj) return;

    obj->ref_count--;

    if (obj->ref_count > 0) return;

    switch (obj->kind) {
        case OBJ_VECTOR3:
            rc_dec(obj->data.vector3.x);
            rc_dec(obj->data.vector3.y);
            rc_dec(obj->data.vector3.z);
            break;

        case OBJ_ARRAY:
            for (size_t i = 0; i < obj->data.array.length; i++) {
                rc_dec(obj->data.array.items[i]);
            }
            break;

        case OBJ_INTEGER:
        case OBJ_FLOAT:
        case OBJ_STRING:
   
            break;
    }
    snake_object_free(obj);
}

void rc_frame_push(VM *vm, snake_object *obj) {
    Frame *frame = vm_current_frame(vm);
    if (!frame) {
        fprintf(stderr, "[RC] Error: rc_frame_push called with no active frame.\n");
        return;
    }
    rc_inc(obj);                   
    frame_push_ref(frame, obj);     
}

void rc_frame_pop(VM *vm) {
    Frame *frame = vm_current_frame(vm);
    if (!frame) {
        fprintf(stderr, "[RC] Error: rc_frame_pop called with no active frame.\n");
        return;
    }
    snake_object *obj = frame_pop_ref(frame);
    rc_dec(obj); 
}

void rc_pop_frame(VM *vm) {
    Frame *frame = vm_pop_frame(vm);
    if (!frame) {
        fprintf(stderr, "[RC] Error: rc_pop_frame called with no frames on stack.\n");
        return;
    }

    for (size_t i = 0; i < frame->references.count; i++) {
        snake_object *obj = (snake_object *)frame->references.items[i];
        rc_dec(obj);
    }

    frame->references.count = 0;
    frame_free(frame);
}

static void mark_gray(snake_object *obj, Stack *gray) {
    if (!obj) return;
    if (obj->is_marked) return; 
    obj->is_marked = true;
    stack_push(gray, obj);
}

void ms_mark(VM *vm, Stack *gray) {
    for (size_t f = 0; f < vm->frames.count; f++) {
        Frame *frame = (Frame *)vm->frames.items[f];

        for (size_t r = 0; r < frame->references.count; r++) {
            snake_object *obj = (snake_object *)frame->references.items[r];
            mark_gray(obj, gray); 
        }
    }
}

void ms_trace(Stack *gray) {
    while (gray->count > 0) {
        snake_object *obj = (snake_object *)stack_pop(gray);

        switch (obj->kind) {
            case OBJ_VECTOR3:
                mark_gray(obj->data.vector3.x, gray);
                mark_gray(obj->data.vector3.y, gray);
                mark_gray(obj->data.vector3.z, gray);
                break;

            case OBJ_ARRAY:
                for (size_t i = 0; i < obj->data.array.length; i++) {
                    mark_gray(obj->data.array.items[i], gray);
                }
                break;

            case OBJ_INTEGER:
            case OBJ_FLOAT:
            case OBJ_STRING:
            
                break;
        }
    }
}

void ms_sweep(VM *vm) {
    size_t new_count = 0; 

    for (size_t i = 0; i < vm->objects.count; i++) {
        snake_object *obj = (snake_object *)vm->objects.items[i];

        if (obj->is_marked) {
            obj->is_marked = false;
            vm->objects.items[new_count++] = obj; 
        } else {
            snake_object_free(obj);
        }
    }
    vm->objects.count = new_count;
}

void ms_collect(VM *vm) {
    Stack gray;
    stack_init(&gray);

    ms_mark(vm, &gray);   
    ms_trace(&gray);     
    ms_sweep(vm);       

    stack_free(&gray);
}