#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#pragma once

// Structure
typedef struct List_Node_t
{
    void *data;
    struct List_Node_t *prev;
    struct List_Node_t *next;
} List_Node_t;

typedef struct
{
    List_Node_t *head;
    List_Node_t *tail;
    uint16_t size; // !shorter length to do positive and negative pos
    uint32_t dsize;
} List_t;

// API
List_t *list_init(uint32_t dsize);
void list_clear(List_t *self);
void list_destroy(List_t *self);
List_t *list_copy(List_t *self);

void *list_front(List_t *self);
void *list_back(List_t *self);
void list_push_front(List_t *self, void *data);
void list_push_back(List_t *self, void *data);
void list_pop_front(List_t *self);
void list_pop_back(List_t *self);
void *list_at(List_t *self, int32_t pos);

void _list_border_(List_t *self, int32_t pos, uintptr_t *left, uintptr_t *right);

// ------------------------------------------------------------------
void list_insert(List_t *self, int32_t pos, void *data);
void list_erase(List_t *self, int32_t pos);
void list_insert_list(List_t *self, int32_t pos, List_t *object);
List_t *list_from_array(void *array, uint32_t asize, uint32_t dsize);
void list_insert_array(List_t *self, int32_t pos, void *array, uint32_t asize);

// ---------------------------------------------------------------------------------------
int32_t list_find_data_range(List_t *self, void *data, uint32_t offset, uint32_t dsize);
int32_t list_find(List_t *self, void *data);

// ------------------------------------ Test --------------------------------------------------
void list_print(List_t *self);