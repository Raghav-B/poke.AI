#
#   Weather update client
#   Connects SUB socket to tcp://localhost:5556
#   Collects weather updates and finds avg temp in zipcode
#

import zmq

#  Socket to talk to server
context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.setsockopt_string(zmq.SUBSCRIBE, "")
socket.setsockopt(zmq.CONFLATE, 1)

print("Collecting ram values from emulator...")
socket.connect("tcp://localhost:5556")

while True:
    string = socket.recv_string()

    #string = str(string)
    vals = [0,0,0,0,0,0]

    for i in range(0, 6):
        if (i + 1) > len(string):
            break
        vals[i] = ord(string[i])
    print(vals)