#!/usr/bin/python           # This is server.py file

import socket               # Import socket module
from random import random
from time import sleep

s = socket.socket()         # Create a socket object
host = socket.gethostname() # Get local machine name
port = 1337                # Reserve a port for your service.

s.connect((host, port))
s.send(b"NAME rand2m\n")

while True:
    sleep(0.1)
    r = int(random() * 6)
    if (r == 0):
        s.send(b"FORWARD\n")
    elif (r == 1):
        s.send(b"BACKWARD\n")
    elif (r == 2):
        s.send(b"STRAFE_RIGHT\n")
    elif (r == 3):
        s.send(b"STRAFE_LEFT\n")
    elif (r == 4):
        s.send(b"FIRE\n")
    elif (r == 5):
        s.send(str.encode("POINT_AT " + str(random() * 1600) + " " + str(random() * 1200) + "\n"))
s.close                     # Close the socket when done
