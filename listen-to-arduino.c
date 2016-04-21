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
#include <sys/socket.h>
#include <time.h>
#define BUFF_SIZE 10000
#define TEMP_MSG_LENGTH 100
#define SIZE_OF_QUEUE 36000

extern float average, min, max;
extern pthread_mutex_t lock;
extern int arduino;
extern int sock;
extern int received;

char test [BUFF_SIZE]; 
double TEMP_TIME_INTERVAL = 1;

void read_message(int arduino, char *buff) {

  // int newline_count = 0;
  int total_bytes = 0;

  // clear local_temp buff
  bzero(buff, TEMP_MSG_LENGTH);

  //Do not exceed reading longer than the length of the msg
  while(!total_bytes || buff[total_bytes - 1] != '\n') {
    //Read only one byte at a time and only execute block if a byte is received
    total_bytes += read(arduino, &buff[total_bytes], 1);
  }
}

void* listen_to_arduino(void* _) {

  // configure connection to Arduino

  //Specific to macs and our team's arduino device
  arduino = open("/dev/cu.usbmodem1411", O_RDWR);

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


  queue_t* q = new_queue(SIZE_OF_QUEUE);
  int num = 0; // for computing average
  char buff [BUFF_SIZE]; // holds strings read from Arduino
  tcflush(arduino, TCIFLUSH); // flush waiting Arduino input
  time_t last_temp_transmission = time(NULL);
  while(1) {
    // pull data from Arduino and enqueue
    // save string from Ardunito to buff
    read_message(arduino, buff); 
    float temp;
    char msg;
    switch (buff[0]) {
      case 't': // temp
        temp = atof(buff + 3);
        enqueue(q, temp);

        // update min, max, and average
        num++;
        pthread_mutex_lock(&lock);
        get_extrema(q, &min, &max); // get min and max values in queue
        average = (average * (num - 1) + temp) / num; // update average
        /*printf("\ntemp: %f\nmin: %f\nmax: %f\naverage: %f\n",*/
            /*temp, min, max, average);*/
        if (difftime(time(NULL), last_temp_transmission) > TEMP_TIME_INTERVAL) {
          puts("\n* Missed temperature transmission from arduino. *\n");
        }
        last_temp_transmission = time(NULL);
        pthread_mutex_unlock(&lock);
        break;
      case 'a': 
        switch (buff[3]) {
        case 'a': // armed
          send(sock, buff, 1, 0);
          break;
        case 't': // triggered
          send(sock, buff, 1, 0);
          break;
        case 'd': // disarmed
          send(sock, buff, 1, 0);
          break;
        case 's': // sounded
          send(sock, buff, 1, 0);
          break;
        default:
          perror(buff);
          exit(1);
          break;
      }
      case 'r': // received message
        pthread_mutex_lock(&lock);
        received = 1;
        pthread_mutex_unlock(&lock);
        break;
      default:
        perror(buff);
        exit(1);
        break;
    }
  }
  close(arduino);
  delete_queue(q);
  return NULL;
}
