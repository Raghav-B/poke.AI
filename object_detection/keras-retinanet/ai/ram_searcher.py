import os
import zmq
# This object will basically run a unix command that checks the virtual memory for a particular process
# We use this to keep track of our character's x and y positions for collision detection
class ram_searcher:
    def __init__(self):
        # Placeholder values, can just ignore since they will be overwritten when new object is created
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        print("Listening to emulator...")
    
    def get_vals(self):
        self.socket.connect("tcp://localhost:5556")
        self.socket.setsockopt_string(zmq.SUBSCRIBE, "")
        string = self.socket.recv_string()
        vals = [0,0,0,0,0]

        for i in range(0, 5):
            if (i + 1) > len(string):
                break
            vals[i] = ord(string[i])
        print(vals)

        self.socket.disconnect("tcp://localhost:5556")
        return vals