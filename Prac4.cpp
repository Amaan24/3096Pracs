/*
 * Prac4.cpp
 * 
 * Originall written by Stefan Schröder and Dillion Heald
 * 
 * Adapted for EEE3096S 2019 by Keegan Crankshaw
 * 
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Prac4.h"
#include <wiringPi.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
using namespace std;

bool playing = true; // should be set false when paused
bool stopped = false; // If set to true, program should close
unsigned char buffer[2][BUFFER_SIZE][2];
int buffer_location = 0;
bool bufferReading = 0; //using this to switch between column 0 and 1 - the first column
bool threadReady = false; //using this to finish writing the first column at the start of the song, before the column is played
long lastInterruptTime = 0; //Used for button debounce

// Configure your interrupts here.
// Don't forget to use debouncing.
void play_pause_isr(void){ //interrupt and debouncing for play/pause
    //Write your logic here
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
	playing = !playing; //toggles between pause and play

}
	lastInterruptTime = interruptTime;
}
void stop_isr(void){ //interrupt and debouncing for stop
    // Write your logic here

	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
	stopped = true; //in STOP state
}
	lastInterruptTime = interruptTime;

}

/*
 * Setup Function. Called once 
 */
int setup_gpio(void){
    //Set up wiring Pi
    wiringPiSetup();
    //setting up the buttons
	//TODO
	pinMode(PLAY_BUTTON, INPUT); //configures buttons to be inputs
	pinMode(STOP_BUTTON, INPUT);
	pullUpDnControl(PLAY_BUTTON, PUD_UP); //sets pull up resistors
	pullUpDnControl(STOP_BUTTON,PUD_UP);
	wiringPiISR(PLAY_BUTTON, INT_EDGE_RISING, &play_pause_isr); //sets up interrupt for play/pause button 
	wiringPiISR(STOP_BUTTON, INT_EDGE_RISING, &stop_isr); //sets up interrupt on stop button
    //setting up the SPI interface
    //TODO
    wiringPiSPISetup (SPI_CHAN, SPI_SPEED); //configures SPI with appropriate channel and speed
    return 0;
}

/* 
 * Thread that handles writing to SPI
 * 
 * You must pause writing to SPI if not playing is true (the player is paused)
 * When calling the function to write to SPI, take note of the last argument.
 * You don't need to use the returned value from the wiring pi SPI function
 * You need to use the buffer_location variable to check when you need to switch buffers
 */
void *playThread(void *threadargs){
    // If the thread isn't ready, don't do anything
    while(!threadReady)
        continue;
    
    //You need to only be playing if the stopped flag is false
    while(!stopped){
        //Code to suspend playing if paused
		//TODO
        
        //Write the buffer out to SPI
        //TODO
	if (playing==true){
	 wiringPiSPIDataRW (SPI_CHAN, buffer[bufferReading][buffer_location], 2); //reads and writes to SPI
		
        //Do some maths to check if you need to toggle buffers
        buffer_location++;
        if(buffer_location >= BUFFER_SIZE) {
            buffer_location = 0;
            bufferReading = !bufferReading; // switches column one it finishes one column
        }
      }//i added this one, delete if wrong
    }
    
    pthread_exit(NULL);
}

int main(){
    // Call the setup GPIO function
	if(setup_gpio()==-1){
        return 0;
    }
    
    /* Initialize thread with parameters
     * Set the play thread to have a 99 priority
     * Read https://docs.oracle.com/cd/E19455-01/806-5257/attrib-16/index.html
     */ 
    
    //Write your logic here
	pthread_attr_t tattr;
    pthread_t thread_id;
    int newprio = 99;
    sched_param param;
    
    pthread_attr_init (&tattr);
    pthread_attr_getschedparam (&tattr, &param); /* safe to get existing scheduling param */
    param.sched_priority = newprio; /* set the priority; others are unchanged */
    pthread_attr_setschedparam (&tattr, &param); /* setting the new scheduling param */
    pthread_create(&thread_id, &tattr, playThread, (void *)1); /* with new priority specified *
    
    /*
     * Read from the file, character by character
     * You need to perform two operations for each character read from the file
     * You will require bit shifting
     * 
     * buffer[bufferWriting][counter][0] needs to be set with the control bits
     * as well as the first few bits of audio
     * 
     * buffer[bufferWriting][counter][1] needs to be set with the last audio bits
     * 
     * Don't forget to check if you have pause set or not when writing to the buffer
     * 
     */
     
    // Open the file
    char ch;
    FILE *filePointer;
    printf("%s\n", FILENAME);
    filePointer = fopen(FILENAME, "r"); // read mode

    if (filePointer == NULL) {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    int counter = 0;
    int bufferWriting = 0;

    // Have a loop to read from the file
	 while((ch = fgetc(filePointer)) != EOF){
        while(threadReady && bufferWriting==bufferReading && counter==0){
            //waits in here after it has written to a side, and the thread is still reading from the other side
            continue;
        }
	if (playing==true){
        //Set config bits for first 8 bit packet and OR with upper bits
        buffer[bufferWriting][counter][0] = 0b01110000 | ch>>6  ; //sets control bits and current bits
        //Set next 8 bit packet
        buffer[bufferWriting][counter][1] = ch << 2  ; //sets next bits

        counter++;
        if(counter >= BUFFER_SIZE+1){
            if(!threadReady){
                threadReady = true;
            }

            counter = 0;
            bufferWriting = (bufferWriting+1)%2;
        }
}//i added this
    }
     
    // Close the file
    fclose(filePointer);
    printf("Complete reading"); 
	 
    //Join and exit the playthread
	pthread_join(thread_id, NULL); 
    pthread_exit(NULL);
	
    return 0;
}

