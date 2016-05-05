
/*
isr4pi.c
http://cs.smith.edu/dftwiki/index.php/Tutorial:_Interrupt-Driven_Event-Counter_on_the_Raspberry_Pi

D. Thiebaut
based on isr.c from the WiringPi library, authored by Gordon Henderson
https://github.com/WiringPi/WiringPi/blob/master/examples/isr.c

Compile as follows:

    gcc -o isr4pi isr4pi.c -lwiringPi

Run as follows:

    sudo ./isr4pi

 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <wiringPi.h>


// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define RAIN_GAUGE_GPIO 6


// the event counter 
volatile int bucket_dump_counter = 0, debounce_counter = 20, last_state, current_state;

struct timeval current_tv, last_tv;
// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void rain_gauge_interrupt(void) {
 
  gettimeofday(&current_tv, 0);
  long elapsed = (current_tv.tv_sec-last_tv.tv_sec)*1000000 + current_tv.tv_usec-last_tv.tv_usec;
  printf("%ld\n",elapsed);
 if (elapsed > 200000 )
 {
 
  while ( debounce_counter > 0 )
    {
    current_state = digitalRead(RAIN_GAUGE_GPIO);
    if ( current_state == last_state )
       { debounce_counter--; }
    else {
       debounce_counter = 5;
       last_state = current_state;
       }
    delay(10);
    }
    
     debounce_counter =  20;
     bucket_dump_counter++;
     last_tv.tv_sec = current_tv.tv_sec;
     last_tv.tv_usec = current_tv.tv_usec;
 }
}


// -------------------------------------------------------------------------
// main
int main(void) {
  // sets up the wiringPi library
  if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }
  pinMode(RAIN_GAUGE_GPIO, INPUT);
  pullUpDnControl(RAIN_GAUGE_GPIO, PUD_UP);
  // set Pin 17/0 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (RAIN_GAUGE_GPIO, INT_EDGE_BOTH, &rain_gauge_interrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  // display counter value every second.
  while ( 1 ) {
    printf( "%d\n", bucket_dump_counter );
//    eventCounter = 0;
    delay( 1000 ); // wait 1 second
  }

  return 0;
}

