#ifndef VM_H
#define VM_H

#include "stack.h"
#include "frame.h"
#include "snake_object.h"

typedef struct {
    Stack frames;  
    Stack objects;
} VM;

VM    *vm_new(void);
void   vm_free(VM *vm);

void   vm_push_frame(VM *vm, Frame *frame);
Frame *vm_pop_frame(VM *vm);  
Frame *vm_current_frame(VM *vm);

void   vm_track_object(VM *vm, snake_object *obj);

#endif 