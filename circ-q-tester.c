#include <stdlib.h>
#include <stdio.h>
#include "circular-queue.h"

circ_q_t* q;

void enqueue_(long n) {
  enqueue(q, (void*) n);
}

int dequeue_() {
  return (int) dequeue(q);
}

void print(void* v) {
  printf("%d ", (int) v);
}

void print_() {
  print_queue(q, print);
}

int main() {
  q = new_circ_queue(3);
  print_();
  enqueue_(1);
  print_();
  enqueue_(2);
  enqueue_(3);
  print_();
  enqueue_(4);
  print_();
  delete_circ_queue(q);
}
