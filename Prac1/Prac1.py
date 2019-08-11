#!/usr/bin/python3
"""
Names: Amaan Vally
Student Number: VLLAMA004
Prac: Prac 1
Date: 03/08/2019
"""

# import Relevant Librares
import RPi.GPIO as GPIO
import itertools

#Assign pins to list for easy manipulation
chan = [36,38,40]

#Configure GPIO pins
GPIO.setmode(GPIO.BOARD)
GPIO.setup(chan,GPIO.OUT, initial = GPIO.LOW)
GPIO.setup(16, GPIO.IN, pull_up_down = GPIO.PUD_UP)
GPIO.setup(18, GPIO.IN, pull_up_down = GPIO.PUD_UP)

#Declare global variables and generate list of binary numbers
count = 0
a = list(itertools.product([0,1], repeat = 3))

#callback functions for button pressed
def button1(channel):
	global a
	global count
	print("ok")
	if count < 7:
		count += 1
		GPIO.output(chan, a[count])
	else:
		count = 0
		GPIO.output(chan, a[count])
def button2(channel):
	global a
	global count
	print("ok2")
	if count > 0:
		count -= 1
		GPIO.output(chan, a[count])
	else:
		count = 7
		GPIO.output(chan, a[count])
#Configure event detection and debouncing
GPIO.add_event_detect(16, GPIO.FALLING, callback = button1, bouncetime=300)
GPIO.add_event_detect(18, GPIO.FALLING, callback = button2, bouncetime=300)


# Logic that you write
def main():
	print("")


# Only run the functions if 
if __name__ == "__main__":
    # Make sure the GPIO is stopped correctly
    try:
        while True:
            main()
    except KeyboardInterrupt:
        print("Exiting gracefully")
        # Turn off your GPIOs here
        GPIO.cleanup()
    except Exception as e:
        GPIO.cleanup()
        print("Some other error occurred")
        print(e.message)
