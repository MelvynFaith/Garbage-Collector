#define _POSIX_C_SOURCE 200809L

#include "snake_object.h"

#include <stdio.h>   
#include <stdlib.h>
#include <string.h>  

static snake_object *alloc_object(snake_object_kind kind) {
    snake_object *obj = malloc(sizeof(snake_object));
    if (!obj) {
        fprintf(stderr, "[Object] Fatal: malloc failed for new object.\n");
        exit(EXIT_FAILURE);
    }
    obj->kind      = kind;
    obj->ref_count = 0; 
    obj->is_marked = false; 
    return obj;
}

snake_object *snake_object_new_integer(int value) {
    snake_object *obj = alloc_object(OBJ_INTEGER);
    obj->data.integer = value;
    return obj;
}

snake_object *snake_object_new_float(double value) {
    snake_object *obj = alloc_object(OBJ_FLOAT);
    obj->data.float_val = value;
    return obj;
}

snake_object *snake_object_new_string(const char *value) {
    snake_object *obj = alloc_object(OBJ_STRING);
    obj->data.string = strdup(value); 
    if (!obj->data.string) {
        fprintf(stderr, "[Object] Fatal: strdup failed.\n");
        exit(EXIT_FAILURE);
    }
    return obj;
}

snake_object *snake_object_new_vector3(snake_object *x,
                                       snake_object *y,
                                       snake_object *z) {
    snake_object *obj = alloc_object(OBJ_VECTOR3);
    obj->data.vector3.x = x;
    obj->data.vector3.y = y;
    obj->data.vector3.z = z;
    return obj;
}

snake_object *snake_object_new_array(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = 4;

    snake_object *obj = alloc_object(OBJ_ARRAY);
    obj->data.array.items    = malloc(initial_capacity * sizeof(snake_object *));
    if (!obj->data.array.items) {
        fprintf(stderr, "[Object] Fatal: array backing malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    obj->data.array.length   = 0;
    obj->data.array.capacity = initial_capacity;
    return obj;
}

void snake_array_push(snake_object *arr, snake_object *item) {
    if (arr->kind != OBJ_ARRAY) {
        fprintf(stderr, "[Object] Error: snake_array_push called on non-array.\n");
        return;
    }

    if (arr->data.array.length == arr->data.array.capacity) {
        arr->data.array.capacity *= 2;
        arr->data.array.items = realloc(arr->data.array.items,
            arr->data.array.capacity * sizeof(snake_object *));
        if (!arr->data.array.items) {
            fprintf(stderr, "[Object] Fatal: array realloc failed.\n");
            exit(EXIT_FAILURE);
        }
    }

    arr->data.array.items[arr->data.array.length++] = item;
}

void snake_object_print(const snake_object *obj) {
    if (!obj) {
        printf("<null>");
        return;
    }

    printf("snake_object@%p { kind=", (void *)obj);

    switch (obj->kind) {
        case OBJ_INTEGER:
            printf("INTEGER, value=%d", obj->data.integer);
            break;

        case OBJ_FLOAT:
            printf("FLOAT, value=%g", obj->data.float_val);
            break;

        case OBJ_STRING:
            printf("STRING, value=\"%s\"", obj->data.string);
            break;

        case OBJ_VECTOR3:
            printf("VECTOR3, x=%p, y=%p, z=%p",
                   (void *)obj->data.vector3.x,
                   (void *)obj->data.vector3.y,
                   (void *)obj->data.vector3.z);
            break;

        case OBJ_ARRAY:
            printf("ARRAY, length=%zu, items=[", obj->data.array.length);
            for (size_t i = 0; i < obj->data.array.length; i++) {
                printf("%p", (void *)obj->data.array.items[i]);
                if (i + 1 < obj->data.array.length) printf(", ");
            }
            printf("]");
            break;
    }

    printf(", ref_count=%d, is_marked=%s }\n",
           obj->ref_count,
           obj->is_marked ? "true" : "false");
}

void snake_object_free(snake_object *obj) {
    if (!obj) return;

    switch (obj->kind) {
        case OBJ_STRING:
            free(obj->data.string);     
            obj->data.string = NULL;
            break;

        case OBJ_ARRAY:
            free(obj->data.array.items);  
            obj->data.array.items    = NULL;
            obj->data.array.length   = 0;
            obj->data.array.capacity = 0;
            break;

        case OBJ_INTEGER:
        case OBJ_FLOAT:
        case OBJ_VECTOR3:
            break;
    }

    free(obj); 
}