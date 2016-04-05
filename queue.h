#ifndef Q_H
#define Q_H
typedef struct {
  float* array;
  float* ptr;
  int size;
  int num_elements;
} queue_t;

queue_t* new_queue(int size);
void delete_queue(queue_t* q);
void enqueue(queue_t* q, float value);
float dequeue(queue_t* q);
void print_queue(queue_t*);
void get_extrema(queue_t*, float*, float*);
int is_full(queue_t* q);
#endif
