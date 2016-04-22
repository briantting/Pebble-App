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
#include <netinet/in.h>
#include <time.h>
#define BUFF_SIZE 10000
#define TEMP_MSG_LENGTH 100
#define SIZE_OF_QUEUE 36000

extern float average, min, max;
extern pthread_mutex_t lock;
extern int arduino;
extern int sock;
extern int received;
extern struct sockaddr_in server_addr;

char* DEVICE = "/dev/cu.usbmodem1411";
double TEMP_TIME_INTERVAL = 2;

int open_device() {

  struct termios options; // struct to hold options
  tcgetattr(arduino, &options); // associate with this fd
  cfsetispeed(&options, 9600); // set input baud rate
  cfsetospeed(&options, 9600); // set output baud rate
  tcsetattr(arduino, TCSANOW, &options); // set options
  return open(DEVICE, O_RDWR);
}

void send_to_pebble(char* msg) {

  server_addr.sin_port = htons(3001); // specify port number
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server_addr.sin_zero),8); 

  // open connectino to pebble
  int fd = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
  if (fd == -1) {
    perror("connect");
    exit(1);
  } 

  send(fd, msg, strlen(msg), 0);
  close(fd);
}

void read_message(int arduino, char *buff) {

  // int newline_count = 0;
  int total_bytes = 0;

  int connection = 1;

  // clear local_temp buff
  bzero(buff, TEMP_MSG_LENGTH);

  //Do not exceed reading longer than the length of the msg
  while(!total_bytes || buff[total_bytes - 1] != '\n') {
    if (connection) {
      //Read only one byte at a time and only execute block if a byte is received
      int bytes_read = read(arduino, &buff[total_bytes], 1);
      if (bytes_read == -1) { 
        connection = 0;
        char* msg = "\n* Lost connection with Arduino. *\n";
        puts(msg);
        send_to_pebble(msg);
      } else {
        connection = 1;
        total_bytes += bytes_read;
      }
    } else {
      arduino = open_device();
      if (arduino != -1) {
        connection = 1;
        char* msg = "\n* Regained connection. *\n";
        puts(msg);
        send_to_pebble(msg);
      }
    }
  }
}

void* listen_to_arduino(void* _) {

  // configure connection to Arduino

  //Specific to computer and arduino device
  arduino = open_device();

  //Exits thread if there was an issue
  if(arduino == -1) {
    perror("There was a problem accessing the Arduino.");
    exit(1);  //May want to consider a non-NULL error type
  }
  

  queue_t* q = new_queue(SIZE_OF_QUEUE);
  int num = 0; // for computing average
  char buff [BUFF_SIZE]; // holds strings read from Arduino
  tcflush(arduino, TCIFLUSH); // flush waiting Arduino input
  int lost_connection = 0;
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
        pthread_mutex_unlock(&lock);

        printf("."); // print dots to signify temp reading
        fflush(stdout);
        break;
      case 'a': 
        send_to_pebble(buff);
      case 'r': // received message
        pthread_mutex_lock(&lock);
        received = 1;
        pthread_mutex_unlock(&lock);
        break;
    }
  }

  close(arduino);
  delete_queue(q);
  return NULL;
}
