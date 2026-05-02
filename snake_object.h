#ifndef SNAKE_OBJECT_H
#define SNAKE_OBJECT_H

#include <stdbool.h> 
#include <stddef.h>  

typedef enum {
    OBJ_INTEGER, 
    OBJ_FLOAT,   
    OBJ_STRING,  
    OBJ_VECTOR3, 
    OBJ_ARRAY,  
} snake_object_kind;

typedef struct snake_object snake_object;

typedef union {
    int integer;

    double float_val;

    char *string;

    struct {
        snake_object *x;
        snake_object *y;
        snake_object *z;
    } vector3;

    struct {
        snake_object **items;   
        size_t         length;   
        size_t         capacity; 
    } array;

} snake_object_data;


struct snake_object {
    snake_object_kind kind;    
    snake_object_data data;    

    int  ref_count;           

    bool is_marked; 
};

snake_object *snake_object_new_integer(int value);
snake_object *snake_object_new_float(double value);
snake_object *snake_object_new_string(const char *value);

snake_object *snake_object_new_vector3(snake_object *x,
                                       snake_object *y,
                                       snake_object *z);
snake_object *snake_object_new_array(size_t initial_capacity);

void snake_array_push(snake_object *arr, snake_object *item);

void snake_object_print(const snake_object *obj);

void snake_object_free(snake_object *obj);

#endif 