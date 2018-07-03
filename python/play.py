#!/usr/bin/env python

from os import listdir
import subprocess
from time import sleep
import RPi.GPIO as GPIO

button_pin = 22
led_pin = 27
music_dir = '../music'

GPIO.setmode(GPIO.BCM)
GPIO.setup(button_pin, GPIO.IN)
GPIO.setup(led_pin, GPIO.OUT)

# turn button LED on while no sound is playing
GPIO.output(led_pin, GPIO.HIGH)

state = GPIO.input(button_pin)
index = 0

wav_files = [ f for f in listdir(music_dir) if f[-4:] == '.wav' ]
if not (len(wav_files) > 0):
    print "No files found!"
    exit()

print '--- Available wav files ---'
print wav_files
print '--- Press button to play next file. ---'

while True:
    current_state = GPIO.input(button_pin)

    if (current_state != state):
        state = current_state
        GPIO.output(led_pin, GPIO.LOW)
        subprocess.Popen(['aplay', wav_files[index]])
        print '--- Playing ' + wav_files[index] + ' ---'
        GPIO.output(led_pin, GPIO.HIGH)
        index += 1
        if index >= len(wav_files):
            index = 0

    sleep(0.1);
