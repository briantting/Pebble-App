#include "listen-to-arduino.h"
#include "queue.h"
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <pthread.h>
#define BUFF_SIZE 10000
#define TEMP_MSG_LENGTH 100
#define SECS_PER_HOUR 3600

extern float average, min, max;
extern pthread_mutex_t lock;
extern int arduino;

char test [BUFF_SIZE]; 

void read_temperature(int arduino, char *temp_buff) {

  // int newline_count = 0;
  int total_bytes = 0;

  // clear local_temp buff
  bzero(temp_buff, TEMP_MSG_LENGTH);

  //Do not exceed reading longer than the length of the msg
  while(!total_bytes || temp_buff[total_bytes - 1] != '\n') {
    //Read only one byte at a time and only execute block if a byte is received
    total_bytes += read(arduino, &temp_buff[total_bytes], 1);
  }
}

void* listen_to_arduino(void* _) {

  // configure connection to Arduino

  //Specific to macs and our team's arduino device
  arduino = open("/dev/cu.usbmodemFD121", O_RDWR);

  //Exits thread if there was an issue
  if(arduino == -1) {
    fprintf(stderr, "%s\n", "There was a problem accessing the Arduino.");
    exit(1);  //May want to consider a non-NULL error type
  }
  
  struct termios options; // struct to hold options
  tcgetattr(arduino, &options); // associate with this fd
  cfsetispeed(&options, 9600); // set input baud rate
  cfsetospeed(&options, 9600); // set output baud rate
  tcsetattr(arduino, TCSANOW, &options); // set options


  queue_t* q = new_queue(SECS_PER_HOUR);
  int num = 0; // for computing average
  char temp_buff [BUFF_SIZE]; // holds strings read from Arduino
  tcflush(arduino, TCIFLUSH); // flush waiting Arduino input
  while(1) {

    // check if user input 'q' to quit
    /*fd_set set;*/
    /*FD_ZERO(&set);*/
    /*FD_SET(0, &set);*/
    /*FD_SET(arduino, &set);*/
    /*if(select(arduino + 1, &set, NULL, NULL, NULL) == -1) {*/
        /*perror("select");*/
        /*exit(1);*/
    /*}*/
    /*if (FD_ISSET(0, &set)) { */
      /*char buff [BUFF_SIZE]; */
      /*fgets(buff, sizeof(buff), stdin); */
      /*if (strcmp(buff, "q\n") == 0) {*/
        /*break; */
      /*}*/
    /*}*/

    // pull data from Arduino and enqueue
    // save string from Ardunito to temp_buff
    read_temperature(arduino, temp_buff); 
    float temp = atof(temp_buff);
    enqueue(q, temp);

    // update min, max, and average
    num++;
    pthread_mutex_lock(&lock);
    get_extrema(q, &min, &max); // get min and max values in queue
    average = (average * (num - 1) + temp) / num; // update average
    printf("\ntemp: %f\nmin: %f\nmax: %f\naverage: %f\n",
        temp, min, max, average);
    pthread_mutex_unlock(&lock);
  }
  close(arduino);
  delete_queue(q);
  return NULL;
}
