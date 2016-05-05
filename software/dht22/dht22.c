/*
dht22.c
2014-09-20
Public Domain

This program reads the relative humidity and temperature
from a DHT22 sensor.

gcc -o dht22 dht22.c
sudo ./dht22 [gpio [tries]]

The gpio defaults to 22 and tries to 5.
*/

#define DHT22 22
#define TRIES 5

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#define GPIO_BASE  0x20200000
#define SYST_BASE  0x20003000

#define GPIO_LEN  0xB4
#define SYST_LEN  0x1C

#define GPSET0 7
#define GPSET1 8

#define GPCLR0 10
#define GPCLR1 11

#define GPLEV0 13
#define GPLEV1 14

#define SYST_CS  0
#define SYST_CLO 1
#define SYST_CHI 2

static volatile uint32_t  *gpioReg = MAP_FAILED;
static volatile uint32_t  *systReg = MAP_FAILED;

#define PI_BANK (gpio>>5)
#define PI_BIT  (1<<(gpio&0x1F))

#define PI_OK      0
#define PI_FAILURE 1

/* gpio levels */

#define PI_LOW  0
#define PI_HIGH 1

/* gpio modes. */

#define PI_INPUT  0
#define PI_OUTPUT 1
#define PI_ALT0   4
#define PI_ALT1   5
#define PI_ALT2   6
#define PI_ALT3   7
#define PI_ALT4   3
#define PI_ALT5   2

void gpioSetMode(unsigned gpio, unsigned mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}

int gpioRead(unsigned gpio)
{
   if ((*(gpioReg + GPLEV0 + PI_BANK) & PI_BIT) != 0) return 1;
   else                                               return 0;
}

void gpioWrite(unsigned gpio, unsigned level)
{
   if (level == 0) *(gpioReg + GPCLR0 + PI_BANK) = PI_BIT;
   else            *(gpioReg + GPSET0 + PI_BANK) = PI_BIT;
}

/* Returns the number of microseconds after system boot. Wraps around
   after 1 hour 11 minutes 35 seconds.
*/

uint32_t gpioTick(void) { return systReg[SYST_CLO]; }

/* Map in registers. */

static uint32_t * initMapMem(int fd, uint32_t addr, uint32_t len)
{
    return (uint32_t *) mmap(0, len,
       PROT_READ|PROT_WRITE|PROT_EXEC,
       MAP_SHARED|MAP_LOCKED,
       fd, addr);
}

int gpioInitialise(void)
{
   int fd;

   fd = open("/dev/mem", O_RDWR | O_SYNC) ;

   if (fd < 0)
   {
      fprintf(stderr,
         "This program needs root privileges.  Try using sudo\n");
      return -1;
   }

   gpioReg  = initMapMem(fd, GPIO_BASE, GPIO_LEN);
   systReg  = initMapMem(fd, SYST_BASE, SYST_LEN);

   close(fd);

   if ((gpioReg == MAP_FAILED) || (systReg == MAP_FAILED))
   {
      fprintf(stderr, "Bad, mmap failed\n");
      return -1;
   }
   return 0;
}

#define MAXEDGES 85

static int read_DHT22(int gpio)
{
   float temp, rhum;
   int lastLevel = PI_HIGH;
   int bit = 0, byte = 0, i;
   uint32_t tick, edgeLen;
   uint8_t msg[5], checksum;

   /* Pull gpio down for 10 milliseconds. */

   gpioWrite(gpio, PI_LOW);
   gpioSetMode(gpio, PI_OUTPUT);
   usleep(10000);

   /* Force high. */
   gpioWrite(gpio, PI_HIGH);

   // Float high. */
   gpioSetMode(gpio, PI_INPUT);

   for (i=0; i<MAXEDGES; i++)
   {
      tick = gpioTick();

      while (gpioRead(gpio) == lastLevel)
      {
         edgeLen = gpioTick() - tick;
         if (edgeLen > 150) break;
      }

      if (edgeLen > 150) break;

      if (lastLevel) lastLevel = 0; else lastLevel = 1;

      /* Ignore first 3 edges. */
      if ((i >= 4) && (i%2 == 0))
      {
         msg[byte] <<= 1;

         if (edgeLen > 50) msg[byte] |= 1;

         if (++bit > 7)
         {
            bit = 0;
            ++byte;
         }
      }
   }

   checksum = msg[0] + msg[1] + msg[2] + msg[3];

   if ((byte >= 5) && (msg[4] == checksum))
   {
      rhum = (float) ((msg[0]<<8) + msg[1]);
      rhum /= 10.0;

      temp = (float) (((msg[2] & 0x7F) << 8) + msg[3]);
      temp /= 10.0;
      if ((msg[2] & 0x80))  temp *= -1.0;

      printf("%.1f %.1f\n", rhum, temp);
      return 1;
   }
   return 0;
}

int main (int argc, char *argv[])
{
   int DHT22_gpio, tries;
   int i;

   if (argc > 1) DHT22_gpio = atoi(argv[1]);
   else          DHT22_gpio = DHT22;

   if (argc > 2) tries = atoi(argv[2]);
   else          tries = TRIES;

   if (argc > 3)
   {
      fprintf(stderr, "Too many arguments.\n");
      return PI_FAILURE;
   }

   if (gpioInitialise() < 0) return PI_FAILURE;

   setuid(getuid()); /* Drop root privileges. */

   i = 0;

   while (1)
   {
      if (read_DHT22(DHT22_gpio)) break;

      if (i >= tries)
      {
         printf("999.9 999.9\n");
         break;
      }

      ++i;
      usleep(2000000); /* Try again in 2 seconds. */
   }
   return 0;
}
