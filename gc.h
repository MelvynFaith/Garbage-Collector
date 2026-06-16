#ifndef GC_H
#define GC_H

#include "vm.h"
#include "frame.h"
#include "snake_object.h"

/* ----------------------------------------------------------------------------
 * Two garbage collectors share one object model (snake_object).
 *
 *   RC  (rc_*) — reference counting. Frees eagerly when ref_count hits 0.
 *                Cannot reclaim reference cycles. RC objects are NOT registered
 *                in vm->objects.
 *   MS  (ms_*) — mark-and-sweep. Reclaims cycles. Every MS object MUST be
 *                registered via vm_track_object(); ms_sweep only frees objects
 *                found in vm->objects.
 *
 * CONTRACT: a given snake_object belongs to exactly ONE collector for its whole
 * lifetime. Driving the same object through both (RC frees it, then MS sweeps
 * it, or vice versa) is a double-free. Pick one per object and stay consistent.
 * -------------------------------------------------------------------------- */

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