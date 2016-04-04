#include "heap.h"
#include <stdio.h>

heap_t* heap;

void insert_(long x) {
  insert(heap, (void*) x);
}

int extract_() {
  return (int) extract(heap);
}

int int_compare(void* a, void* b) {
  return (int) a - (int) b;
}

void print_extracted() {
  int extracted = extract_();
  printf("Extracted = %d\n", extracted);
}

void print(void* x) {
  printf("%d", (int) x);
}

int main() {
  heap = new_heap(1, int_compare);
  insert_(5);
  print_heap(heap, print);
  insert_(3);
  print_heap(heap, print);
  print_extracted();
  insert_(3);
  print_heap(heap, print);
  insert_(2);
  print_heap(heap, print);
  insert_(4);
  print_heap(heap, print);
  insert_(5);
  insert_(5);
  insert_(5);
  insert_(35);
  insert_(1);
  insert_(5);
  print_heap(heap, print);
  insert_(5);
  print_heap(heap, print);
  insert_(8);
  print_heap(heap, print);
  insert_(9);
  print_heap(heap, print);
  insert_(4);
  print_heap(heap, print);
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  print_extracted();
  delete_heap(heap);
}
