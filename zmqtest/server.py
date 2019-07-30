import zmq
import time
import random

ctrl_context = zmq.Context()
ctrl_socket = ctrl_context.socket(zmq.REP)
ctrl_socket.bind("tcp://*:5555")

ram_context = zmq.Context()
ram_socket = ram_context.socket(zmq.SUB)
#timeout = 1
#ram_socket.setsockopt(zmq.RCVTIMEO, timeout)

ram_socket.setsockopt_string(zmq.SUBSCRIBE, "")

i = 0

rand = 0
#for i in range(0, 5):
while True:
    #rand = random.randint(0,3)

    data_req = ctrl_socket.recv()
    # Whenever I send a movement key, I'll also send a request to get back data from the RAM search
    send = None
    if rand == 0:
        ctrl_socket.send(chr(0b01000000).encode("utf-8"))
    elif rand == 1:
        ctrl_socket.send(chr(0b00100000).encode("utf-8"))
    elif rand == 2:
        ctrl_socket.send(chr(0b00010000).encode("utf-8"))
    else:
        ctrl_socket.send(chr(0b00001000).encode("utf-8"))

    ram_socket.connect("tcp://localhost:5556")

    string = ram_socket.recv_string()
    vals = [0,0,0,0,0,0]

    for i in range(0, 6):
        if (i + 1) > len(string):
            break
        vals[i] = ord(string[i])
    print(vals)

    ram_socket.disconnect("tcp://localhost:5556")

    i += 1
    print(i)

    time.sleep(5)

ctrl_context.destroy()

