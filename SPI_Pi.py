# SPI communication between Raspberry Pi and PIC16F887

import spidev
import sys
import time
import RPi.GPIO as GPIO

spi = spidev.SpiDev(0, 0)
spi.mode = 2
spi.max_speed_hz = 7629


def readPIC():
    r = spi.xfer2([0x00])
    return r


if __name__ == '__main__':
    try:

        while True:

            cont = spi.xfer2([0x03])
            print("Numero recibido fue: " + str(cont))

            time.sleep(0.5)

    except KeyboardInterrupt:
        spi.close()
        sys.exit(0)

    finally:
        GPIO.cleanup()
