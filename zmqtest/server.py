import zmq
import time

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5555")

while True:
    #request = socket.recv()

    controls = 0b10000000
    
    send = chr(controls).encode("utf-8")
    
    for i in range(10):
        socket.send(send)
        #socket.send(send)
    #print("Data sent received.")

    time.sleep(5)
