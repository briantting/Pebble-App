#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "queue.h"


queue_t* new_queue(int size) {
  queue_t* q = malloc(sizeof(queue_t));
  q->array = calloc(size, sizeof(float));
  q->ptr = q -> array;
  q->size = size;
  q->num_elements = 0;
  return q;
}

void shift_ptr(queue_t* q, int amount) {
  int i = q->ptr - q->array;
  int j = (i + amount) % q->size;
  q->ptr = q->array + j; 
}

void delete_queue(queue_t* q) {
  free(q->array);
  free(q);
}

void enqueue(queue_t* q, float value) {
  q->ptr[0] = value;
  shift_ptr(q, 1);
  if (q->num_elements == q->size) {
    puts("Warning: overwrote the tail element of the queue.");
  }
  if (q->num_elements < q->size) {
    q->num_elements++;
  }
}

int is_full(queue_t* q) {
  return q->num_elements == q->size;
}

float dequeue(queue_t* q) {
  shift_ptr(q, -1);
  return q->ptr[0];
  q->num_elements--;
}


void get_extrema(queue_t* q,
                 float* min_ptr,
                 float* max_ptr) {
  float min = FLT_MAX;
  float max = FLT_MIN;
  for (int i = 0; i < q->num_elements; i++) {
    float val = q->ptr[i];
    if (val > max) {
      max = val;
    }
    if (val < min) {
      min = val;
    }
  }
  *min_ptr = min;
  *max_ptr = max;
}

void print_queue(queue_t* q) {
  for (int i = 0; i < q->size; i++) {
    printf("[%d]: %f\n", i, q->array[i]);
  }
  printf("\n");
}
