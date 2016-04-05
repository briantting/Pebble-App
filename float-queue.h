#ifndef Q_H
#define Q_H
typedef struct {
  void** array;
  void** ptr;
  int size;
} queue_t;

queue_t* new_queue(int size);
void delete_queue(queue_t* q);
void enqueue(queue_t* q, void* value);
void* dequeue(queue_t* q);
void print_queue(queue_t*, void(*)(void*));
void get_extrema(queue_t*, int(*)(void*, void*), void**, void**);
#endif
