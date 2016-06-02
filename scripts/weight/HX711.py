#!/usr/bin/env python

# HX711.py
# 2016-05-01
# Public Domain
# Modified for Hivetool.org usage
# Version 1.1

CH_A_GAIN_64  = 0 # Channel A gain 64
CH_A_GAIN_128 = 1 # Channel A gain 128
CH_B_GAIN_32  = 2 # Channel B gain 32

import time

import pigpio # http://abyz.co.uk/rpi/pigpio/python.html

class sensor:

   """
   A class to read the HX711 24-bit ADC.
   """

   def __init__(self, pi, DATA, CLOCK, mode=CH_A_GAIN_128, callback=None):
      """
      Instantiate with the Pi, the data GPIO, and the clock GPIO.

      Optionally the channel and gain may be specified with the
      mode parameter as follows.

      CH_A_GAIN_64  - Channel A gain 64
      CH_A_GAIN_128 - Channel A gain 128
      CH_B_GAIN_32  - Channel B gain 32

      Optionally a callback to be called for each new reading may be
      specified.  The callback receives three parameters, the count,
      the mode, and the reading.  The count is incremented for each
      new reading.
      """
      self.pi = pi
      self.DATA = DATA
      self.CLOCK = CLOCK
      self.set_mode(mode)
      self.callback = callback

      self._paused = False
      self._data_level = 0
      self._clocks = -1
      self._value = 0
      self._reading = None
      self._count = 0

      pi.set_mode(CLOCK, pigpio.OUTPUT)
      pi.set_mode(DATA, pigpio.INPUT)

      pi.write(CLOCK, 1) # Pause the sensor.

      pi.wave_add_generic(
         [pigpio.pulse(1<<CLOCK, 0, 20), pigpio.pulse(0, 1<<CLOCK, 20)])

      self._wid = pi.wave_create()

      self._cb1 = pi.callback(DATA, pigpio.EITHER_EDGE, self._callback)
      self._cb2 = pi.callback(CLOCK, pigpio.FALLING_EDGE, self._callback)

      self._valid_after = time.time() + 0.4

      pi.write(CLOCK, 0) # Start the sensor.

   def get_reading(self):
      """
      Returns the current count, mode, and reading.

      The count is incremented for each new reading.
      """
      return self._count, self._mode, self._reading

   def set_callback(self, callback):
      """
      Sets the callback to be called for every new reading.
      The callback receives three parameters, the count,
      the mode, and the reading.  The count is incremented
      for each new reading.

      The callback can be cancelled by passing None.
      """
      self.callback = callback

   def set_mode(self, mode):
      """
      Sets the mode.

      CH_A_GAIN_64  - Channel A gain 64
      CH_A_GAIN_128 - Channel A gain 128
      CH_B_GAIN_32  - Channel B gain 32
      """
      self._mode = mode

      if mode == CH_A_GAIN_128:
         self._pulses = 25
      elif mode == CH_B_GAIN_32:
         self._pulses = 26
      elif mode == CH_A_GAIN_64:
         self._pulses = 27
      else:
         raise ValueError

      self._valid_after = time.time() + 0.4

   def get_mode(self):
      """
      Returns the current mode.
      """
      return self._mode

   def pause(self):
      """
      Pauses readings.
      """
      self._paused = True
      self.pi.wave_tx_stop()
      self.pi.write(self.CLOCK, 1)
      self.pi.set_watchdog(self.CLOCK, 0) # Cancel any timeout.

   def start(self):
      """
      Starts readings.
      """
      self.pi.write(self.CLOCK, 0)
      self._clocks = -1
      self._value = 0
      self._paused = False
      self._valid_after = time.time() + 0.4

   def cancel(self):
      """
      Cancels the sensor and release resources.
      """
      if self._cb1 is not None:
         self._cb1.cancel()
         self._cb1 = None

      if self._cb2 is not None:
         self._cb2.cancel()
         self._cb2 = None

      if self._wid is not None:
         self.pi.wave_delete(self._wid)
         self._wid = None

      self.pi.set_watchdog(self.CLOCK, 0) # cancel timeout

   def _callback(self, gpio, level, tick):

      if gpio == self.CLOCK:

         if level == 0:

            self._clocks += 1

            if self._clocks < 25:

               self._value = (self._value << 1) + self._data_level

         else: #  timeout

            self.pi.set_watchdog(self.CLOCK, 0) # cancel timeout

            if self._clocks == self._pulses:
               if self._value & 0x800000:
                  self._value |= ~0xffffff

               if not (self._paused) and (time.time() > self._valid_after):
                  self._reading = self._value
                  self._count += 1
                  if self.callback is not None:
                     self.callback(self._count, self._mode, self._reading)

            self._clocks = 0
            self._value = 0

      else:

         self._data_level = level

         if (level == 0) and (self._clocks == 0):

            if not self._paused:
               self.pi.wave_chain([255, 0, self._wid, 255, 1, self._pulses, 0])
               self.pi.set_watchdog(self.CLOCK, 2) # 2 ms timeout

if __name__ == "__main__":

   import time
   import pigpio
   import HX711

   #def cbf(count, mode, reading):
   #  print(count, mode, reading)

   pi = pigpio.pi()
   if not pi.connected:
      exit(0)

   #print("start with CH_B_GAIN_32 and callback")

   s = HX711.sensor(pi, DATA=23, CLOCK=24, mode=CH_A_GAIN_128, callback=None)

   time.sleep(5)

   #s.set_mode(CH_A_GAIN_64)

   #print("Change mode to CH_A_GAIN_64")

   #time.sleep(5)

   #s.set_mode(CH_A_GAIN_128)

   #print("Change mode to CH_A_GAIN_128")

   #time.sleep(5)

   #s.pause()

   #print("Pause")

   #time.sleep(5)

   s.start()

   #print("Start")

   time.sleep(5)

   #s.set_callback(None)

   #print("Cancel callback and read manually")

   stop = time.time() + 5

   c, m, r = s.get_reading()

   print(r)

   #while time.time() < stop:
    #  count, mode, reading = s.get_reading()
     # if count != c:
      #   c = count
       #  print(mode, reading)
      #time.sleep(0.01)

   s.pause()

   s.cancel()

   pi.stop()