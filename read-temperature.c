#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#define TEMP_MSG_LENGTH 100


/*******************************************************************************
  GLOBAL VARIABLES
*******************************************************************************/

/*
  The Arduino message is "The temperature is 28.1875 degree C\n\n"
  This is 37 characters long
  The CONSTANT def will allow the length to change and minimize space/time complexity
  The lock prevents reading from the buffer while it is cleared and writing input
*/
// extern pthread_mutex_t lock;

/*******************************************************************************
  FUNCTIONS
*******************************************************************************/

/*
  Thread function that continously reads temperature output from
  Arduino hooked up to Mac and stores in global buffer
*/
void read_temperature(char *temp_buff) {

  //Specific to macs and our team's arduino device
  int fd = open("/dev/cu.usbmodem1411", O_RDWR);

  //Exits thread if there was an issue
  if(fd == -1) {
    fprintf(stderr, "%s\n", "There was a problem accessing the Arduino.");
    return NULL;  //May want to consider a non-NULL error type
  }
  
  struct termios options; // struct to hold options
  tcgetattr(fd, &options); // associate with this fd
  cfsetispeed(&options, 9600); // set input baud rate
  cfsetospeed(&options, 9600); // set output baud rate
  tcsetattr(fd, TCSANOW, &options); // set options

  // int newline_count = 0;
  int total_bytes = 0;
  // int bytes_read = 0;
  // char newline_tester;

  //Continously read and fill buffer
  // char local_temp[TEMP_MSG_LENGTH];
  // while(1) { //****update boolean to terminate when server is terminated
    
    //Reset counting variables
    /*newline_count = 0;*/
    total_bytes = 0;

    // clear local_temp buff
    bzero(temp_buff, TEMP_MSG_LENGTH);

    //Do not exceed reading longer than the length of the msg
    while(temp_buff[total_bytes - 1] != '\n') {
      //Read only one byte at a time and only execute block if a byte is received
      total_bytes += read(fd, &temp_buff[total_bytes], 1);
    }
    // pthread_mutex_lock(&lock); //protects buffer

    // char* local_temp_ptr = local_temp;
    // strsep(&local_temp_ptr, "\n");
    // bzero(temperature, TEMP_MSG_LENGTH);
    // strcpy(temperature, local_temp);

    // pthread_mutex_unlock(&lock);
  // }
}
