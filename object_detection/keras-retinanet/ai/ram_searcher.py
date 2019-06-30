import os

class ram_searcher:
    game_pid = 5446
    x_pos_address = 0x55b2dc683720
    y_pos_address = 0x55b2dc683722

    def __init__(self, pid, xpa, ypa):
        self.game_pid = pid
        self.x_pos_address = xpa
        self.y_pos_address = ypa
    
    def get_x_pos(self):
        x_pos = os.popen('sudo dd bs=1 skip="$((' + str(self.x_pos_address) + '))" count=4 if="/proc/' + \
        str(self.game_pid) + '/mem" | od -An -vtu4').read()
        x_pos = x_pos[4:-1]
        return int(x_pos)
    
    def get_y_pos(self):
        y_pos = os.popen('sudo dd bs=1 skip="$((' + str(self.y_pos_address) + '))" count=4 if="/proc/' + \
        str(self.game_pid) + '/mem" | od -An -vtu4').read()
        y_pos = y_pos[4:-1]
        return int(y_pos)