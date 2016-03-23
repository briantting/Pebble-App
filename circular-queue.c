#include <stdlib.h>
#include <stdio.h>
#include "circular-queue.h"

circ_q_t* new_circ_queue(int size) {
  circ_q_t* q = malloc(sizeof(circ_q_t));
  q->array = calloc(size, sizeof(void*));
  q->ptr = q -> array;
  q->size = size;
  return q;
}

void shift_ptr(circ_q_t* q, int amount) {
  int i = q->ptr - q->array;
  int j = (i + amount) % q->size;
  q->ptr = q->array + j; 
}

void delete_circ_queue(circ_q_t* q) {
  free(q->array);
  free(q);
}

void enqueue(circ_q_t* q, void* value) {
  q->ptr[0] = value;
  shift_ptr(q, 1);
}

void* dequeue(circ_q_t* q) {
  shift_ptr(q, -1);
  return q->ptr[0];
}

void print_queue(circ_q_t* q, void(*print)(void*)) {
  for (int i = 0; i < q->size; i++) {
    print(q->array[i]);
  }
  printf("\n");
}
