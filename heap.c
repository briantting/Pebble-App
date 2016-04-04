#include "heap.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/////////////DEBUG TOOLS
void printn(int n) {
  printf("\n%d!!!!!!!!!!!!!!!\n", n);
}

void printv(void* v) {
  printf("%d", (int) v);
}
/////////////DEBUG TOOLS

void swap(void** a, void** b) {
  void* temp = *a;
  *a = *b;
  *b = temp;
}

void resize(heap_t* heap) {
  void** new_array = calloc(heap->array_size * 2, sizeof(void*));
  if (!new_array) {
    perror("calloc");
    exit(1);
  }
  size_t size = heap->array_size * sizeof(void*);
  memcpy(new_array, heap->array, size);
  free(heap->array);
  heap->array = new_array;
  heap->array_size = heap->array_size * 2;
}

int generalized_compare(heap_t* heap, void* a, void* b) {
  if (!a && !b) {
    return 0;
  } else if (!a) {
    return -1;
  } else if (!b) {
    return 1;
  } else {
    return heap->compare(a, b);
  }
}

void bubble_up(heap_t* heap) {

  int i = heap->num_elements;
  while (i / 2 > 0) {

    void** ptr = heap->array + i;
    void** parent = heap->array + i / 2;

    // parent has lower priority than *ptr
    if (generalized_compare(heap, *parent, *ptr) < 0) {
      swap(ptr, parent);
      i = i / 2;
    } else {
      break;
    }
  }
}

void insert(heap_t* heap, void* value) {
  if (heap->num_elements + 1 >= heap->array_size) { // if full
    resize(heap);
  }
  heap->array[heap->num_elements + 1] = value; // add value to bottom of heap
  heap->num_elements++;
  bubble_up(heap);
}

void bubble_down(heap_t* heap) {

  int i = 1;
  while (i * 2 < heap->array_size) {

    void** ptr = heap->array + i;
    void** left = ptr + i;
    void** right = left + 1;

    // left branch has lower priority than right
    if (generalized_compare(heap, *left, *right) < 0) {
      swap(ptr, right);
      i = i * 2 + 1;
    } else {
      swap(ptr, left);
      i = i * 2;
    }
  }
}

void* extract(heap_t* heap) {
  if (heap->num_elements == 0) {
    perror("Cannot extract from empty heap.");
    exit(1);
  }
  void* ret_val = heap->array[1];
  heap->array[1] = NULL;
  heap->num_elements--;
  bubble_down(heap);
  return ret_val;
}

heap_t* new_heap(int array_size, int (*compare)(void*, void*)) {
  if (array_size < 1) {
    perror("array_size must be at least 1.");
    exit(1);
  }
  heap_t* heap = malloc(sizeof(heap_t));
  if (!heap) {
    perror("malloc");
    exit(1);
  }
  heap->array = calloc(array_size, sizeof(void*));
  if (!heap->array) {
    perror("calloc");
    exit(1);
  }
  heap->array_size = array_size;
  heap->num_elements = 0;
  heap->compare = compare;
  return heap;
}

void delete_heap(heap_t* heap) {
  free(heap->array);
  free(heap);
}

void print_heap(heap_t* heap, void(*print)(void*)) {
  printf("heap array: ");
  int newline = 1;
  for (int i = 1; i <= heap->array_size; i++) {
    if (i == newline) {
      printf("\n");
      newline *= 2;
    }
    print(heap->array[i]);
    printf("|");
  }
  printf("\n");
}
