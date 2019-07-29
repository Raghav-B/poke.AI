import ctypes
import pyautogui as pag
import time
import random
import zmq

from ram_searcher import ram_searcher

W = 0x11
A = 0x1E
S = 0x1F
D = 0x20
SendInput = ctypes.windll.user32.SendInput
# C struct redefinitions 
PUL = ctypes.POINTER(ctypes.c_ulong)
class KeyBdInput(ctypes.Structure):
    _fields_ = [("wVk", ctypes.c_ushort),
                ("wScan", ctypes.c_ushort),
                ("dwFlags", ctypes.c_ulong),
                ("time", ctypes.c_ulong),
                ("dwExtraInfo", PUL)]
class HardwareInput(ctypes.Structure):
    _fields_ = [("uMsg", ctypes.c_ulong),
                ("wParamL", ctypes.c_short),
                ("wParamH", ctypes.c_ushort)]
class MouseInput(ctypes.Structure):
    _fields_ = [("dx", ctypes.c_long),
                ("dy", ctypes.c_long),
                ("mouseData", ctypes.c_ulong),
                ("dwFlags", ctypes.c_ulong),
                ("time",ctypes.c_ulong),
                ("dwExtraInfo", PUL)]
class Input_I(ctypes.Union):
    _fields_ = [("ki", KeyBdInput),
                 ("mi", MouseInput),
                 ("hi", HardwareInput)]
class Input(ctypes.Structure):
    _fields_ = [("type", ctypes.c_ulong),
                ("ii", Input_I)]

class windows_controller:
    def __init__(self, x, y):
        self.corner_x = x # x coordinate of top left corner of gameplay
        self.corner_y = y # y coordinate
        self.key_hold_time = 0.25 # If this is too short, character won't move, will instead turn
        pag.FAILSAFE = False
        pag.PAUSE = 0

        # Bring game window into focus so controls can be sent
        pag.moveTo(self.corner_x + 100, self.corner_y + 100)
        pag.click() # Click on window to focus

    def PressKey(self, hexKeyCode):
        extra = ctypes.c_ulong(0)
        ii_ = Input_I()
        ii_.ki = KeyBdInput( 0, hexKeyCode, 0x0008, 0, ctypes.pointer(extra) )
        x = Input( ctypes.c_ulong(1), ii_ )
        ctypes.windll.user32.SendInput(1, ctypes.pointer(x), ctypes.sizeof(x))
    def ReleaseKey(self, hexKeyCode):
        extra = ctypes.c_ulong(0)
        ii_ = Input_I()
        ii_.ki = KeyBdInput( 0, hexKeyCode, 0x0008 | 0x0002, 0, ctypes.pointer(extra) )
        x = Input( ctypes.c_ulong(1), ii_ )
        ctypes.windll.user32.SendInput(1, ctypes.pointer(x), ctypes.sizeof(x))

    def move_up(self):
        start_time = time.time()
        self.PressKey(W)
        while time.time() - start_time < self.key_hold_time:
            pass
        self.ReleaseKey(W)
        return 0
    def move_right(self):
        start_time = time.time()
        self.PressKey(D)
        while time.time() - start_time < self.key_hold_time:
            pass
        self.ReleaseKey(D)
        return 1
    def move_down(self):
        start_time = time.time()
        self.PressKey(S)
        while time.time() - start_time < self.key_hold_time:
            pass
        self.ReleaseKey(S)
        return 2
    def move_left(self):
        start_time = time.time()
        self.PressKey(A)
        while time.time() - start_time < self.key_hold_time:
            pass
        self.ReleaseKey(A)
        return 3
    def interact(self):
        start_time = time.time()
        self.PressKey("z")
        while time.time() - start_time < self.key_hold_time:
            pass # Change to "x" on ubuntu
        self.ReleaseKey("z")
        return 4

    # A nice test function, by default moves the character randomly, but a string of
    # actions to perform can be sent from main.py
    def perform_movement(self, action=-1):
        if action == -1:
            action = random.randint(0, 3) # Currently removed 4 because it ends up getting stuck in dialogue
        key_pressed = None
        if action == 0:
            key_pressed = self.move_up()
        elif action == 1:
            key_pressed = self.move_right()
        elif action == 2:
            key_pressed = self.move_down()
        elif action == 3:
            key_pressed = self.move_left()
        elif action == 4:
            key_pressed = self.interact()
        return key_pressed

class ubuntu_controller:
    def __init__(self, x, y):
        self.corner_x = x
        self.corner_y = y
        self.key_hold_time = 0.25
        pag.FAILSAFE = False
        pag.PAUSE = 0

        # Bring game window into focus so controls can be sent
        pag.moveTo(self.corner_x + 100, self.corner_y + 100)
        pag.click() # Click on window to focus

    def move_up(self):
        start_time = time.time()
        while time.time() - start_time < self.key_hold_time:
            pag.keyDown("up")
        pag.keyUp("up")
        return 0
    def move_right(self):
        start_time = time.time()
        while time.time() - start_time < self.key_hold_time:
            pag.keyDown("right")
        pag.keyUp("right")
        return 1
    def move_down(self):
        start_time = time.time()
        while time.time() - start_time < self.key_hold_time:
            pag.keyDown("down")
        pag.keyUp("down")
        return 2
    def move_left(self):
        start_time = time.time()
        while time.time() - start_time < self.key_hold_time:
            pag.keyDown("left")
        pag.keyUp("left")
        return 3
    def interact(self):
        start_time = time.time()
        while time.time() - start_time < self.key_hold_time:
            pag.keyDown("z")
        pag.keyUp("z")
        return 4

    # A nice test function, by default moves the character randomly, but a string of
    # actions to perform can be sent from main.py
    def random_movement(self, action=-1):
        if action == -1:
            action = random.randint(1, 3) # Currently removed 4 because it ends up getting stuck in dialogue
        key_pressed = None
        if action == 0:
            key_pressed = self.move_up()
        elif action == 1:
            key_pressed = self.move_right()
        elif action == 2:
            key_pressed = self.move_down()
        elif action == 3:
            key_pressed = self.move_left()
        elif action == 4:
            key_pressed = self.interact()
        return key_pressed

class backend_controller:
    def __init__(self):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PUB)
        self.socket.bind("tcp://*:5555")
        self.key_hold_time = 0.25
        self.ram_search = ram_searcher()
        self.cur_dir = 0

    def get_init_ram_vals(self):
        ram_vals = self.ram_search.get_vals()
        if ram_vals[2] == 1:
            self.cur_dir = 2
        elif ram_vals[2] == 2:
            self.cur_dir = 0
        elif ram_vals[2] == 3:
            self.cur_dir = 3
        elif ram_vals[3] == 4:
            self.cur_dir = 1
        return ram_vals

    def move_up(self):
        self.socket.send(chr(0b10000000).encode("utf-8"))
        return 0
    def move_right(self):
        self.socket.send(chr(0b01000000).encode("utf-8"))
        return 1
    def move_down(self):
        self.socket.send(chr(0b00100000).encode("utf-8"))
        return 2
    def move_left(self):
        self.socket.send(chr(0b00010000).encode("utf-8"))
        return 3
    def interact(self):
        self.socket.send(chr(0b00001000).encode("utf-8"))
        return 4

    # A nice test function, by default moves the character randomly, but a string of
    # actions to perform can be sent from main.py
    def perform_movement(self, action=-1):
        if action == -1:
            action = random.randint(0, 3) # Currently removed 4 because it ends up getting stuck in dialogue
        key_pressed = None

        if action == 0:
            key_pressed = self.move_up()
            time.sleep(self.key_hold_time)
            ram_vals = self.ram_search.get_vals()
            temp = ram_vals[5]
            time.sleep(self.key_hold_time)

            if (self.cur_dir != 0):
                key_pressed = self.move_up()
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
            else:
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
                ram_vals[5] = temp

            self.cur_dir = 0

        elif action == 1:
            key_pressed = self.move_right()
            time.sleep(self.key_hold_time)
            ram_vals = self.ram_search.get_vals()
            temp = ram_vals[5]
            time.sleep(self.key_hold_time)
            
            if (self.cur_dir != 1):
                key_pressed = self.move_right()
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
            else:
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
                ram_vals[5] = temp

            self.cur_dir = 1

        elif action == 2:
            key_pressed = self.move_down()
            time.sleep(self.key_hold_time)
            ram_vals = self.ram_search.get_vals()
            temp = ram_vals[5]
            time.sleep(self.key_hold_time)

            if (self.cur_dir != 2):
                key_pressed = self.move_down()
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
            else:
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
                ram_vals[5] = temp
                
            self.cur_dir = 2

        elif action == 3:
            key_pressed = self.move_left()
            time.sleep(self.key_hold_time)
            ram_vals = self.ram_search.get_vals()
            time.sleep(self.key_hold_time)
            temp = ram_vals[5]
            
            if (self.cur_dir != 3):
                key_pressed = self.move_left()
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
            else:
                time.sleep(self.key_hold_time)
                ram_vals = self.ram_search.get_vals()
                ram_vals[5] = temp

            self.cur_dir = 3

        elif action == 4:
            key_pressed = self.interact()

        print("Current direction: " + str(self.cur_dir))
        time.sleep(self.key_hold_time)
        return key_pressed, ram_vals