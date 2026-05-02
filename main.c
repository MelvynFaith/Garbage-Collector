#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_object.h"
#include "frame.h"
#include "vm.h"
#include "gc.h"

static void print_header(const char *title) {
    printf("\n");
    printf("  %s\n", title);
}

static void print_test(const char *name) {
    printf("  [TEST] %s\n", name);
}

static void print_pass(const char *name) {
    printf("  [PASS] %s\n", name);
}

static snake_object *new_int(VM *vm, int v) {
    snake_object *o = snake_object_new_integer(v);
    vm_track_object(vm, o);
    return o;
}

static snake_object *new_float(VM *vm, double v) {
    snake_object *o = snake_object_new_float(v);
    vm_track_object(vm, o);
    return o;
}

static snake_object *new_str(VM *vm, const char *s) {
    snake_object *o = snake_object_new_string(s);
    vm_track_object(vm, o);
    return o;
}

static snake_object *new_vec3(VM *vm,
                               snake_object *x,
                               snake_object *y,
                               snake_object *z) {
    snake_object *o = snake_object_new_vector3(x, y, z);
    vm_track_object(vm, o);
    return o;
}

static snake_object *new_array(VM *vm, size_t cap) {
    snake_object *o = snake_object_new_array(cap);
    vm_track_object(vm, o);
    return o;
}

static void test_rc_basic_primitives(void) {
    print_test("RC: basic primitives (int, float, string)");

    VM *vm = vm_new();

    vm_push_frame(vm, frame_new());

    snake_object *i = snake_object_new_integer(42);
    snake_object *f = snake_object_new_float(3.14);
    snake_object *s = snake_object_new_string("hello");

    rc_frame_push(vm, i);
    rc_frame_push(vm, f);
    rc_frame_push(vm, s);

    assert(i->ref_count == 1 && "Integer ref_count should be 1");
    assert(f->ref_count == 1 && "Float ref_count should be 1");
    assert(s->ref_count == 1 && "String ref_count should be 1");

    rc_pop_frame(vm);

    assert(vm->frames.count == 0);

    vm_free(vm);
    print_pass("RC: basic primitives");
}

static void test_rc_ref_counting_increment_decrement(void) {
    print_test("RC: manual inc/dec, shared references");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *obj = snake_object_new_integer(99);

    rc_inc(obj);
    rc_inc(obj);  
    rc_inc(obj);  
    assert(obj->ref_count == 3);

    rc_dec(obj); 
    assert(obj->ref_count == 2);

    rc_dec(obj);  
    assert(obj->ref_count == 1);

    rc_dec(obj); 

    Frame *empty = vm_pop_frame(vm);
    frame_free(empty);

    vm_free(vm);
    print_pass("RC: manual inc/dec, shared references");
}

static void test_rc_vector3_cascade(void) {
    print_test("RC: vector3 cascade free of children");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *x = snake_object_new_integer(1);
    snake_object *y = snake_object_new_integer(2);
    snake_object *z = snake_object_new_integer(3);

    snake_object *vec = snake_object_new_vector3(x, y, z);

    rc_inc(x);  
    rc_inc(y);  
    rc_inc(z); 

    rc_frame_push(vm, vec); 

    assert(vec->ref_count == 1);
    assert(x->ref_count == 1);
    assert(y->ref_count == 1);
    assert(z->ref_count == 1);

    printf("    Before frame pop: vec.ref=%d  x.ref=%d  y.ref=%d  z.ref=%d\n",
           vec->ref_count, x->ref_count, y->ref_count, z->ref_count);

    rc_pop_frame(vm);

    printf("    After frame pop: all 4 objects cascade-freed.\n");

    vm_free(vm);
    print_pass("RC: vector3 cascade free");
}

static void test_rc_array_cascade(void) {
    print_test("RC: array cascade free of children");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *arr = snake_object_new_array(4);

    const char *words[] = {"alpha", "beta", "gamma", "delta"};
    for (int i = 0; i < 4; i++) {
        snake_object *s = snake_object_new_string(words[i]);
        rc_inc(s); 
        snake_array_push(arr, s);
    }

    assert(arr->data.array.length == 4);
    for (size_t i = 0; i < arr->data.array.length; i++) {
        assert(arr->data.array.items[i]->ref_count == 1);
    }

    rc_frame_push(vm, arr);
    assert(arr->ref_count == 1);

    rc_pop_frame(vm);

    vm_free(vm);
    print_pass("RC: array cascade free");
}

static void test_rc_cycle_leak_demonstration(void) {
    print_test("RC: cycle leak demonstration (manual cleanup required)");

    snake_object *arr_a = snake_object_new_array(1);
    snake_object *arr_b = snake_object_new_array(1);

    rc_inc(arr_b);
    snake_array_push(arr_a, arr_b);

    rc_inc(arr_a);
    snake_array_push(arr_b, arr_a);

    rc_inc(arr_a); 
    rc_inc(arr_b); 

    assert(arr_a->ref_count == 2); 
    assert(arr_b->ref_count == 2);

    rc_dec(arr_a); 
    rc_dec(arr_b); 

    assert(arr_a->ref_count == 1 && "Cycle: A still has ref from B");
    assert(arr_b->ref_count == 1 && "Cycle: B still has ref from A");

    printf("    CYCLE LEAK: arr_a.ref=%d  arr_b.ref=%d — neither freed!\n",
           arr_a->ref_count, arr_b->ref_count);
    printf("    RC cannot break this cycle. Mark-and-Sweep handles it.\n");
    printf("    Manually freeing both to prevent Valgrind noise...\n");

    arr_a->data.array.items[0] = NULL; 
    arr_b->data.array.items[0] = NULL; 
    snake_object_free(arr_a);
    snake_object_free(arr_b);

    print_pass("RC: cycle leak demonstrated (expected behavior)");
}


static void test_ms_basic_collection(void) {
    print_test("MS: basic collection — unreachable objects swept");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *live  = new_int(vm, 1); 
    snake_object *dead1 = new_int(vm, 2); 
    snake_object *dead2 = new_str(vm, "orphan");

    frame_push_ref(vm_current_frame(vm), live);

    assert(vm->objects.count == 3);

    printf("    Before GC: %zu objects tracked\n", vm->objects.count);

    ms_collect(vm);

    printf("    After GC: %zu objects tracked\n", vm->objects.count);
    assert(vm->objects.count == 1 && "Only 'live' should survive");
    assert(vm->objects.items[0] == live && "Survivor should be 'live'");
    assert(live->is_marked == false && "Marked flag reset after sweep");

    (void)dead1; (void)dead2;

    Frame *f = vm_pop_frame(vm);
    frame_free(f);

    snake_object_free(live);
    vm->objects.count = 0;

    vm_free(vm);
    print_pass("MS: basic collection");
}

static void test_ms_container_reachability(void) {
    print_test("MS: container children reachable through trace");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *x = new_float(vm, 1.0);
    snake_object *y = new_float(vm, 2.0);
    snake_object *z = new_float(vm, 3.0);
    snake_object *vec = new_vec3(vm, x, y, z);

    frame_push_ref(vm_current_frame(vm), vec);

    assert(vm->objects.count == 4);
    ms_collect(vm);

    assert(vm->objects.count == 4 && "vec + 3 children should all survive");
    printf("    After GC: %zu objects alive (vec + 3 children)\n",
           vm->objects.count);

    Frame *f = vm_current_frame(vm);
    f->references.count = 0;

    ms_collect(vm);
    assert(vm->objects.count == 0 && "All 4 objects should now be swept");
    printf("    After 2nd GC: %zu objects alive (all swept)\n",
           vm->objects.count);

    Frame *popped = vm_pop_frame(vm);
    frame_free(popped);
    vm_free(vm);
    print_pass("MS: container children reachable through trace");
}

static void test_ms_array_reachability(void) {
    print_test("MS: array children reachable through trace");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *arr = new_array(vm, 4);
    for (int i = 0; i < 4; i++) {
        snake_object *s = new_str(vm, "item");
        snake_array_push(arr, s);
    }
    assert(vm->objects.count == 5);

    frame_push_ref(vm_current_frame(vm), arr);

    ms_collect(vm);
    assert(vm->objects.count == 5 && "All 5 should survive");

    vm_current_frame(vm)->references.count = 0;
    ms_collect(vm);
    assert(vm->objects.count == 0 && "All 5 swept after losing root");

    Frame *f = vm_pop_frame(vm);
    frame_free(f);
    vm_free(vm);
    print_pass("MS: array children reachable through trace");
}

static void test_ms_cycle_collection(void) {
    print_test("MS: reference cycle collected correctly");

    VM *vm = vm_new();
    vm_push_frame(vm, frame_new());

    snake_object *arr_a = new_array(vm, 1);
    snake_object *arr_b = new_array(vm, 1);

    snake_array_push(arr_a, arr_b); 
    snake_array_push(arr_b, arr_a); 

    assert(vm->objects.count == 2);

    printf("    Cycle created: arr_a=%p → arr_b=%p → arr_a\n",
           (void*)arr_a, (void*)arr_b);
    printf("    Neither is in any frame (no external root).\n");

    ms_collect(vm);

    printf("    After GC: %zu objects alive (expected 0)\n",
           vm->objects.count);
    assert(vm->objects.count == 0 &&
           "Cycle objects must be collected by Mark-and-Sweep");

    Frame *f = vm_pop_frame(vm);
    frame_free(f);
    vm_free(vm);
    print_pass("MS: reference cycle collected");
}

static void test_ms_multi_frame_roots(void) {
    print_test("MS: multiple active frames — all roots preserved");

    VM *vm = vm_new();

    vm_push_frame(vm, frame_new());
    snake_object *outer_obj = new_int(vm, 100);
    frame_push_ref(vm_current_frame(vm), outer_obj);

    vm_push_frame(vm, frame_new());
    snake_object *inner_obj = new_str(vm, "inner scope");
    frame_push_ref(vm_current_frame(vm), inner_obj);

    snake_object *orphan = new_float(vm, 9.99);
    (void)orphan; 

    assert(vm->objects.count == 3);
    assert(vm->frames.count == 2);

    ms_collect(vm);

    assert(vm->objects.count == 2 && "outer + inner must survive");
    printf("    After GC: %zu objects (outer_obj + inner_obj kept, orphan swept)\n",
           vm->objects.count);

    Frame *inner = vm_pop_frame(vm);
    frame_free(inner);

    ms_collect(vm);
    assert(vm->objects.count == 1 && "inner_obj swept after its frame popped");
    assert(vm->objects.items[0] == outer_obj);

    Frame *outer = vm_pop_frame(vm);
    frame_free(outer);

    ms_collect(vm);
    assert(vm->objects.count == 0 && "outer_obj swept after its frame popped");

    vm_free(vm);
    print_pass("MS: multiple active frames");
}

static void test_object_construction_and_print(void) {
    print_test("Object: construction and print for all types");

    snake_object *i   = snake_object_new_integer(42);
    snake_object *f   = snake_object_new_float(2.71828);
    snake_object *s   = snake_object_new_string("SnakeLang");
    snake_object *x   = snake_object_new_integer(10);
    snake_object *y   = snake_object_new_integer(20);
    snake_object *z   = snake_object_new_integer(30);
    snake_object *vec = snake_object_new_vector3(x, y, z);
    snake_object *arr = snake_object_new_array(2);
    snake_array_push(arr, i);
    snake_array_push(arr, s);

    assert(i->kind   == OBJ_INTEGER);
    assert(f->kind   == OBJ_FLOAT);
    assert(s->kind   == OBJ_STRING);
    assert(vec->kind == OBJ_VECTOR3);
    assert(arr->kind == OBJ_ARRAY);
    assert(i->data.integer  == 42);
    assert(arr->data.array.length == 2);
    assert(strcmp(s->data.string, "SnakeLang") == 0);

    printf("\n    --- Object print output ---\n");
    snake_object_print(i);
    snake_object_print(f);
    snake_object_print(s);
    snake_object_print(vec);
    snake_object_print(arr);
    printf("    ----------------------------\n");

    snake_object_free(arr);
    snake_object_free(vec);
    snake_object_free(x);
    snake_object_free(y);
    snake_object_free(z);
    snake_object_free(s);
    snake_object_free(f);
    snake_object_free(i);

    print_pass("Object: construction and print");
}

static void test_stack_operations(void) {
    print_test("Stack: push, pop, peek, resize");

    Stack s;
    stack_init(&s);

    assert(stack_pop(&s) == NULL  && "pop empty returns NULL");
    assert(stack_peek(&s) == NULL && "peek empty returns NULL");

    int vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = i;
        stack_push(&s, &vals[i]);
    }
    assert(s.count == 20);
    assert(s.capacity >= 20);

    assert(*(int *)stack_peek(&s) == 19);

    for (int i = 19; i >= 0; i--) {
        int *v = (int *)stack_pop(&s);
        assert(*v == i);
    }
    assert(s.count == 0);
    assert(stack_pop(&s) == NULL);

    stack_free(&s);
    print_pass("Stack: push, pop, peek, resize");
}

int main(void) {
    print_header("PART 1 — Stack & Object Self-Tests");
    test_stack_operations();
    test_object_construction_and_print();

    print_header("PART 2 — Reference Counting GC");
    test_rc_basic_primitives();
    test_rc_ref_counting_increment_decrement();
    test_rc_vector3_cascade();
    test_rc_array_cascade();
    test_rc_cycle_leak_demonstration();

    print_header("PART 3 — Mark-and-Sweep GC");
    test_ms_basic_collection();
    test_ms_container_reachability();
    test_ms_array_reachability();
    test_ms_cycle_collection();
    test_ms_multi_frame_roots();

    printf("\n");
    printf("  ALL TESTS PASSED\n");

    return EXIT_SUCCESS;
}