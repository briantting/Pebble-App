#include "read-temperature.h"
#include "listen-to-arduino.h"
#include "queue.h"
#include <stdlib.h>
#include <float.h>
#define BUFF_SIZE 10000
#define SECS_PER_HOUR 3600

extern float average, min, max;

void* listen_to_arduino(void* argv) {
  queue_t* q = new_queue(SECS_PER_HOUR);
  int num = 0;
  while(1) {
    char temp_buff [BUFF_SIZE] = "test";
    read_temperature(temp_buff);
    float temp = atoi(temp_buff);
    enqueue(q, temp);
    get_extrema(q, &min, &max);
    num++;
    average = (average * (num - 1) + temp) / num;
  }
  delete_queue(q);
}
