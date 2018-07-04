#!/usr/bin/env python

from os import listdir
import subprocess
from time import sleep
import RPi.GPIO as GPIO
import serial

button_pin = 22
led_pin = 27
music_dir = '/home/pi/shitter/music/'
serial_port = '/dev/ttyUSB0'

esp = serial.Serial(
    port=serial_port,
    baudrate=115200
)

esp.isOpen() or exit(1)

GPIO.setmode(GPIO.BCM)
GPIO.setup(button_pin, GPIO.IN)
GPIO.setup(led_pin, GPIO.OUT)

# turn button LED on while no sound is playing
GPIO.output(led_pin, GPIO.HIGH)

state = GPIO.input(button_pin)
index = 0

wav_files = [f for f in listdir(music_dir) if f[-4:] == '.wav']
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

        # turn button LED off while playing
        GPIO.output(led_pin, GPIO.LOW)

        # tell ESP to start patterns
        esp.write(chr(1))

        print '--- Playing ' + wav_files[index] + ' ---'
        subprocess.call(['aplay', '-B', '10000', '-R', '0', music_dir + wav_files[index]])

        # turn button LED back on
        GPIO.output(led_pin, GPIO.HIGH)

        # tell ESP to go back to idle mode
        esp.write(chr(0))

        # loop through playlist
        index += 1
        if index >= len(wav_files):
            index = 0
    else:
        sleep(0.1)
