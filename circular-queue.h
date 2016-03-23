#ifndef CIRC_Q_H
#define CIRC_Q_H
typedef struct {
  void** array;
  void** ptr;
  int size;
} circ_q_t;

circ_q_t* new_circ_queue(int size);
void delete_circ_queue(circ_q_t* q);
void enqueue(circ_q_t* q, void* value);
void* dequeue(circ_q_t* q);
void print_queue(circ_q_t*, void(*)(void*));
#endif
