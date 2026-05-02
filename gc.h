#ifndef GC_H
#define GC_H

#include "vm.h"
#include "frame.h"
#include "snake_object.h"

void rc_inc(snake_object *obj);

void rc_dec(snake_object *obj);

void rc_frame_push(VM *vm, snake_object *obj);

void rc_frame_pop(VM *vm);

void rc_pop_frame(VM *vm);

void ms_collect(VM *vm);

void ms_mark(VM *vm, Stack *gray);

void ms_trace(Stack *gray);

void ms_sweep(VM *vm);

#endif 