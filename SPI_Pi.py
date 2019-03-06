# SPI communication between Raspberry Pi and PIC16F887

import spidev
import sys
import time
import RPi.GPIO as GPIO

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 1953000
spi.cshigh = False
spi.bits_per_word = 8

try:

    while True:

        val = spi.xfer2([0x00])
        print("Numero recibido fue: " + str(val))
        time.sleep(1)

except KeyboardInterrupt:
    spi.close()
    sys.exit(0)

finally:
    GPIO.cleanup()
