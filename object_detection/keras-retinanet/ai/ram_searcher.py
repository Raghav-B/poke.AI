import os
import zmq
# This object will basically run a unix command that checks the virtual memory for a particular process
# We use this to keep track of our character's x and y positions for collision detection
class ram_searcher:
    # Placeholder values, can just ignore since they will be overwritten when new object is created
    context = None
    socket = None

    #game_pid = 5446
    #x_pos_address = 0x55b2dc683720
    #y_pos_address = 0x55b2dc683722

    def __init__(self):#, pid, xpa, ypa):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        print("Listening to emulator...")
        
        #self.game_pid = pid
        #self.x_pos_address = xpa
        #self.y_pos_address = ypa
    
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

    # Get x_pos
    def get_x_pos(self):
        #x_pos = os.popen('sudo dd bs=1 skip="$((' + str(self.x_pos_address) + '))" count=4 if="/proc/' + \
        #str(self.game_pid) + '/mem" | od -An -vtu4').read()
        #x_pos = x_pos[4:-1]
        return 0#int(x_pos)
    
    # Get y_pos
    def get_y_pos(self):
        #y_pos = os.popen('sudo dd bs=1 skip="$((' + str(self.y_pos_address) + '))" count=4 if="/proc/' + \
        #str(self.game_pid) + '/mem" | od -An -vtu4').read()
        #y_pos = y_pos[4:-1]
        return 0#int(y_pos)

    # TODO: Clean output of the two functions above