#
#   Weather update client
#   Connects SUB socket to tcp://localhost:5556
#   Collects weather updates and finds avg temp in zipcode
#

import zmq

#  Socket to talk to server
context = zmq.Context()
socket = context.socket(zmq.SUB)

print("Collecting ram values from emulator...")
socket.connect("tcp://localhost:5556")

socket.setsockopt_string(zmq.SUBSCRIBE, "")

while True:
    string = socket.recv_string()

    #string = str(string)
    vals = [0,0,0,0,0]

    for i in range(0, 5):
        if (i + 1) > len(string):
            break
        vals[i] = ord(string[i])

    #if (len(string) == 2):
    #    string[2] = '\0'
    #elif (len(string) == 1):
    #    string[1] = '\0'
    #    string[2] = '\0'
    #print(f"x_pos: {ord(string[0])}, y_pos: {ord(string[1])}, dir: {ord(string[2])}")
    print(vals)