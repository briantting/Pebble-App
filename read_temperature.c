#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#define TEMP_MSG_LENGTH 37


/*******************************************************************************
	GLOBAL VARIABLES
*******************************************************************************/

/*
	The Arduino message is "The temperature is 28.1875 degree C\n\n"
	This is 37 characters long
	The CONSTANT def will allow the length to change and minimize space/time complexity
	The lock prevents reading from the buffer while it is cleared and writing input
*/
extern char* temperature;
extern pthread_mutex_t lock;

/*******************************************************************************
	FUNCTIONS
*******************************************************************************/

/*
	Resets the read_temperature buffer to be entirely blank
*/
void clear_buffer() {
    for (int i = 0; i < TEMP_MSG_LENGTH; i++) {
        temperature[i] = '\0';
    }    
}

/*
	Thread function that continously reads temperature output from
	Arduino hooked up to Mac and stores in global buffer
*/
void* read_temperature(void *p) {

	//Specific to macs and our team's arduino device
	int fd = open("/dev/cu.usbmodem1411", O_RDWR);

	//Exits thread if there was an issue
	if(fd == -1) {
		fprintf(stderr, "%s\n", "There was a problem accessing the Arduino.");
		return NULL; 	//May want to consider a non-NULL error type
	}
	
	struct termios options; // struct to hold options
	tcgetattr(fd, &options); // associate with this fd
	cfsetispeed(&options, 9600); // set input baud rate
	cfsetospeed(&options, 9600); // set output baud rate
	tcsetattr(fd, TCSANOW, &options); // set options

	int newline_count = 0;
	int total_bytes = 0;
	int bytes_read = 0;
	char newline_tester;

	//Find the beginning of a new reading by getting fd to end of the line
	while(newline_count != 2) {
		if((bytes_read = read(fd, &newline_tester, 1)) != 0) {
				if(newline_tester == '\n') {
					newline_count += 1;
				} 
		}
	}

	//Continously read and fill buffer
	while(1) { //****update boolean to terminate when server is terminated
		pthread_mutex_lock(&lock); //protects buffer
		
		clear_buffer(); 

		//Reset counting variables
		newline_count = 0;
		total_bytes = 0;

		//Do not exceed reading longer than the length of the msg
		while(total_bytes != TEMP_MSG_LENGTH || newline_count != 2) {
			//Read only one byte at a time and only execute block if a byte is received
			if((bytes_read = read(fd, &temperature[total_bytes], 1)) != 0) {
				total_bytes += bytes_read; 
				if(temperature[total_bytes - 1] == '\n') {
					newline_count += 1;
				} 
			}
		}
		printf("%s", temperature);	//for testing output
		pthread_mutex_unlock(&lock);
    puts("sleeping");
    sleep(2);
    puts("waking");

	}
}

/*******************************************************************************
	MAIN MOTHERF$#@ER
*******************************************************************************/

/*int main()*/
/*{	*/
	/*pthread_t thread;*/
	/*pthread_create(&thread, NULL, &read_temperature, NULL);*/
	/*pthread_join(thread, NULL);*/
	/*return 0;*/
/*}*/



