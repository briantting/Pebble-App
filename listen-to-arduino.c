#include "read-temperature.h"
#include "listen-to-arduino.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <pthread.h>
#define BUFF_SIZE 10000
#define SECS_PER_HOUR 3600

extern float average, min, max;
extern pthread_mutex_t lock;

void* listen_to_arduino(void* argv) {
  queue_t* q = new_queue(SECS_PER_HOUR);
  int num = 0;
  char temp_buff [BUFF_SIZE];
  while(1) {
    read_temperature(temp_buff);
    float temp = atoi(temp_buff);
    enqueue(q, temp);

    num++;
    pthread_mutex_lock(&lock);
    get_extrema(q, &min, &max);
    average = (average * (num - 1) + temp) / num;
    printf("temp: %f\nmin: %f\nmax: %f\naverage: %f",
        temp, min, max, average);
    pthread_mutex_unlock(&lock);
  }
  delete_queue(q);
}
