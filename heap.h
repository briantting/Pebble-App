typedef struct {
  void** array;
  int array_size;
  int num_elements;
  int (*compare)(void*, void*);
} heap_t;

void swap(void**, void**);
void resize(heap_t*);
void bubble_up(heap_t*);
void insert(heap_t*, void*);
void bubble_down(heap_t*);
void* extract(heap_t*);
heap_t* new_heap(int, int (*)(void*, void*));
void delete_heap(heap_t*);
int int_compare(void*, void*);
void print_heap(heap_t*, void(*)(void*));
