#!/usr/bin/env python

import os
from time import sleep
import RPi.GPIO as GPIO

button_pin = 22
led_pin = 27

GPIO.setmode(GPIO.BCM)
GPIO.setup(button_pin, GPIO.IN)
GPIO.setup(led_pin, GPIO.OUT)

# turn button LED on while no sound is playing
GPIO.output(led_pin, GPIO.HIGH)

state = GPIO.input(button_pin)

while True:
    current_state = GPIO.input(button_pin)

    if (current_state != state):
        state = current_state
        GPIO.output(led_pin, GPIO.LOW)
        os.system('aplay ../music/the\ great\ flush.wav')
        GPIO.output(led_pin, GPIO.HIGH)

    sleep(0.1);
