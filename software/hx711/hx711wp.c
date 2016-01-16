/* 
 gurov was here, use this code, or don't, whatever, I don't care. If you see a giant bug with a billion legs, please let me know so it can be squashed
gcc -o hx711wp hx711wp.c -lwiringPi
*/

#include <stdint.h>
#include <sys/types.h>


#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "locking.h"


#define CLOCK_PIN	5
#define DATA_PIN	4
#define N_SAMPLES	64
#define SPREAD		10

#define SCK_ON  (GPIO_SET0 = (1 << CLOCK_PIN))
#define SCK_OFF (GPIO_CLR0 = (1 << CLOCK_PIN))
#define DT_R    (GPIO_IN0  & (1 << DATA_PIN))

void           reset_converter(void);
unsigned long  read_cnt(long offset, int argc);
void           set_gain(int r);
void           setHighPri (void);
int data_pin = DATA_PIN;
int clock_pin = CLOCK_PIN;


void setHighPri (void)
{
  struct sched_param sched ;

  memset (&sched, 0, sizeof(sched)) ;

  sched.sched_priority = 10 ;
  if (sched_setscheduler (0, SCHED_FIFO, &sched))
    printf ("Warning: Unable to set high priority\n") ;
}

static uint8_t sizecvt(const int read)
{
    /* digitalRead() and friends from wiringpi are defined as returning a value
       < 256. However, they are returned as int() types. This is a safety function */

    if (read > 255 || read < 0)
    {
        printf("Invalid data from wiringPi library\n");
        exit(EXIT_FAILURE);
    }
    return (uint8_t)read;
}


void power_down_hx711()
{
    digitalWrite(clock_pin, HIGH);
}

void setup_gpio()
{
    pinMode(clock_pin, OUTPUT);
    pinMode(data_pin, INPUT);
    digitalWrite(clock_pin, LOW);
}


int main(int argc, char **argv)
{
    int lockfd;
    int iPin = 0;
    int iErr = 0;
    char cMode = 'V';
    int iReturnCode = 0;


  int i, j;
  long tmp=0;
  long tmp_avg=0;
  long tmp_avg2;
  long offset=0;
  float filter_low, filter_high;
  float spread_percent = SPREAD / 100.0 /2.0;
  int b;
  int nsamples=N_SAMPLES;
  long samples[nsamples];

  if (argc == 2) {
   offset = atol(argv[1]);
  }
  if (argc == 4){
   clock_pin = atol(argv[1]);
   data_pin = atol(argv[2]);
   offset = atol(argv[3]);
  }
//    lockfd = open_lockfile(LOCKFILE);

        iErr = wiringPiSetup ();
    if (iErr == -1)
        {
                if (cMode == 'V')
                        printf ("ERROR : Failed to init WiringPi %d\n", iErr);
        iReturnCode = -1;
        }

  setHighPri();
  setup_gpio();
//  reset_converter();

  j=0;

  // get the dirty samples and average them
  for(i=0;i<nsamples;i++) {
 // 	reset_converter();
 //       reset_converter();
  	samples[i] = read_cnt(0, argc);
  	tmp_avg += samples[i];
  }

  tmp_avg = tmp_avg / nsamples;

  tmp_avg2 = 0;
  j=0;

  filter_low =  (float) tmp_avg * (1.0 - spread_percent);
  filter_high = (float) tmp_avg * (1.0 + spread_percent);

//  printf("%d %d\n", (int) filter_low, (int) filter_high);

  for(i=0;i<nsamples;i++) {
	if ((samples[i] < filter_high && samples[i] > filter_low) || 
            (samples[i] > filter_high && samples[i] < filter_low) ) {
		tmp_avg2 += samples[i];
	        j++;
	}
  }

  if (j == 0) {
    printf("No data to consider\n");
    exit(255);

  }
  printf("%d\n", (tmp_avg2 / j) - offset);

//  printf("average within %f percent: %d from %d samples, original: %d\n", spread_percent*100, (tmp_avg2 / j) - offset, j, tmp_avg - offset);
//  unpull_pins();
//  restore_io();
//  power_down_hx711();
}


void reset_converter(void) {
          digitalWrite(clock_pin, HIGH);
          delayMicroseconds(40);
          digitalWrite(clock_pin, LOW);
          delayMicroseconds(60);
}

void set_gain(int r) {
	int i;

// r = 0 - 128 gain ch a
// r = 1 - 32  gain ch b
// r = 2 - 63  gain ch a

        while( sizecvt(digitalRead(data_pin)) );

	for (i=0;i<24+r;i++) {
          digitalWrite(clock_pin, HIGH);
          delayMicroseconds(1);
          digitalWrite(clock_pin, LOW);
          delayMicroseconds(1);
	}
}


unsigned long read_cnt(long offset, int argc) {
	long count;
	int i;


  count = 0;


  while( sizecvt(digitalRead(data_pin)) );
        digitalWrite(clock_pin, LOW);
        delayMicroseconds(.2);

  for(i=0;i<24	; i++) {
        digitalWrite(clock_pin, HIGH);
        delayMicroseconds(.2);
        count = count << 1;
        if ( sizecvt(digitalRead(data_pin)) > 0 )  { count++; }
        digitalWrite(clock_pin, LOW);
        delayMicroseconds(.2);
        }

        digitalWrite(clock_pin, HIGH);
        delayMicroseconds(.2);
        digitalWrite(clock_pin, LOW);
        delayMicroseconds(.2);

//  count = ~0x1800000 & count;
//  count = ~0x800000 & count;


 if (count & 0x800000) {
	count |= (long) ~0xffffff;
 }

// if things are broken this will show actual data


if (argc < 2 ) {
 for (i=32;i;i--) {
   printf("%d ", ((count-offset) & ( 1 << i )) > 0 );
  }

  printf("n: %10d     -  ", count - offset);
  printf("\n"); 
}
  return (count - offset);

}


