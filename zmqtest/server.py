#
#   Weather update server
#   Binds PUB socket to tcp://*:5556
#   Publishes random weather updates
#

import zmq
from random import randrange

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5556")

string = "helloboi"

while True:
    #zipcode = randrange(1, 100000)
    #temperature = randrange(-80, 135)
    #relhumidity = randrange(10, 60)


    print("Now sending...")

    socket.send_string("helloboi")