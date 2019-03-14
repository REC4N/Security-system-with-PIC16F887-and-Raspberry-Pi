# SPI communication between Raspberry Pi and PIC16F887
# Authors: Alejandro Recancoj and Julio Shin

import spidev
import sys
import time
import RPi.GPIO as GPIO
from Adafruit_IO import Client, Feed

run_count = 0

# Set to your Adafruit IO key.
# Remember, your key is a secret,
# so make sure not to publish it when you publish this code!
ADAFRUIT_IO_KEY = '96756677c7e6420ca405a832ce8d532e'

# Set to your Adafruit IO username.
# (go to https://accounts.adafruit.com to find your username)
ADAFRUIT_IO_USERNAME = 'edgar_rec'

# Create an instance of the REST client.
aio = Client(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)

spi = spidev.SpiDev()					# SPI communication is opened.
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 1953000
spi.cshigh = False
spi.bits_per_word = 8
data = [0, 0, 0, 0, 0, 0, 0, 0, 0]		# data array is initialized.

try:

    while True:

        while True:
            a = spi.xfer2([0x00])		#Initiate saving of data until char 'A' is received
            if (chr(a[0]) == 'A'):
                break
            time.sleep(0.005)

        for i in range(0, 9):
            x = spi.xfer2([0x00])
            y = x[0]
            data[i] = y
            time.sleep(0.005)			# Save the 9 bits of data as an arrary.

        temp = chr(data[0]) + chr(data[1]) + chr(data[2]) + chr(data[3]) + chr(data[4])  #The first 5 bits is the temperature data, so it is converted to ascii and then joined to form a string.
        try:							
            temp = float(temp)			# Convert temperature to a float
            print("Temp: " + str(temp) + ". Door: " + str(data[5]) + ". Alarm: " + str(data[6]) + ". Bank: " + str(data[7]) + ". Security: " + str(data[8]))

            val1 = temp                 # val1 is the temperature.
            if (data[5] == 1):			# val2 tell us if the door of the vault is opened or closed.
                val2 = "Opened"
            else:
                val2 = "Closed"
            if (data[8] == 1):			# val3 tell us if the security system is on or off.
                val3 = "ON"
            else:
                val3 = "OFF"
            if (data[6] == 1):			# val4 indicate if the alarm is on or off.
                val4 = "ON"
            else:
                val4 = "OFF"
            if (data[7] == 3):			# val5 indicate if the bank is closed or opened.
                val5 = "Opened"
            else:
                val5 = "Closed"
            aio.send_data('prueba1', val1)		#All the data is sent to Adafruit IO.
            aio.send_data('prueba2', val2)
            aio.send_data('prueba3', val3)
            aio.send_data('prueba4', val4)
            aio.send_data('prueba5', val5)
            time.sleep(11)

        except:					#If an error ocurred, just restart the saving of data.
            pass


except KeyboardInterrupt:				# If Ctrl + C is used, spi communication is terminated and sys is closed.
    spi.close()
    sys.exit(0)

finally:
    GPIO.cleanup()						# GPIO pins are cleared for good measure.
