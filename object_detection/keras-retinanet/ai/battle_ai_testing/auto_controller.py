import pyautogui as pag
import time
import random

# Global variables, these should be inside the class below but I'm too
# lazy to type self. everywhere
corner_x = 0 # x coordinate of top left corner of gameplay
corner_y = 0 # y coordinate
key_hold_time = 0.25 # If this is too short, character won't move, will instead turn
# Storing character facing direction. CURRENTLY NOT IN USE
cur_dir = 0 # 0=up, 1=right, 2=down, 3=left

""" IF WINDOWS """
import ctypes

SendInput = ctypes.windll.user32.SendInput

W = 0x11
A = 0x1E
S = 0x1F
D = 0x20

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

# Actuals Functions

def PressKey(hexKeyCode):
    extra = ctypes.c_ulong(0)
    ii_ = Input_I()
    ii_.ki = KeyBdInput( 0, hexKeyCode, 0x0008, 0, ctypes.pointer(extra) )
    x = Input( ctypes.c_ulong(1), ii_ )
    ctypes.windll.user32.SendInput(1, ctypes.pointer(x), ctypes.sizeof(x))

def ReleaseKey(hexKeyCode):
    extra = ctypes.c_ulong(0)
    ii_ = Input_I()
    ii_.ki = KeyBdInput( 0, hexKeyCode, 0x0008 | 0x0002, 0, ctypes.pointer(extra) )
    x = Input( ctypes.c_ulong(1), ii_ )
    ctypes.windll.user32.SendInput(1, ctypes.pointer(x), ctypes.sizeof(x))

class controller:
    def __init__(self, x, y):
        pag.FAILSAFE = False
        pag.PAUSE = 0
        self.corner_x = x
        self.corner_y = y

        # Bring game window into focus so controls can be sent
        pag.moveTo(self.corner_x + 100, self.corner_y + 100)
        pag.click() # Click on window to focus
    

    def move_up(self):
        start_time = time.time()
        PressKey(W)
        while time.time() - start_time < key_hold_time:
            pass
        ReleaseKey(W)
        cur_dir = 0
        return 0


    def move_right(self):
        start_time = time.time()
        PressKey(D)
        while time.time() - start_time < key_hold_time:
            pass
        ReleaseKey(D)
        cur_dir = 1
        return 1


    def move_down(self):
        start_time = time.time()
        PressKey(S)
        while time.time() - start_time < key_hold_time:
            pass
        ReleaseKey(S)
        cur_dir = 2
        return 2


    def move_left(self):
        start_time = time.time()
        PressKey(A)
        while time.time() - start_time < key_hold_time:
            pass
        ReleaseKey(A)
        cur_dir = 3
        return 3


    def interact(self):
        start_time = time.time()
        PressKey("z")
        while time.time() - start_time < key_hold_time:
            pass # Change to "x" on ubuntu
        ReleaseKey("z") # Change to "x" on ubuntu
        return 4 # Change to "x" on ubuntu

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

    # Dummy function that does absolutely nothing
    def dummy(self):
        pag.press("")
    
    def win_test(self):
        self.move_up()
        return 0

""" IF UBUNTU """
"""
# The controller object is based on pyautogui and is used to send commands to the game
# as player input
class controller:
    def __init__(self, x, y):
        pag.FAILSAFE = False
        pag.PAUSE = 0
        self.corner_x = x
        self.corner_y = y

        # Bring game window into focus so controls can be sent
        pag.moveTo(self.corner_x + 100, self.corner_y + 100)
        pag.click() # Click on window to focus
    

    def move_up(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("up")
        ReleaseKey("up")
        cur_dir = 0
        return "up"


    def move_right(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("right")
        ReleaseKey("right")
        cur_dir = 1
        return "right"


    def move_down(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("down")
        ReleaseKey("down")
        cur_dir = 2
        return "down"


    def move_left(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("left")
        ReleaseKey("left")
        cur_dir = 3
        return "left"


    def interact(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("z") # Change to "x" on ubuntu
        ReleaseKey("z") # Change to "x" on ubuntu
        return "z" # Change to "x" on ubuntu

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

    # Dummy function that does absolutely nothing
    def dummy(self):
        pag.press("")
    
    def win_test(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("up")
        ReleaseKey("up")
        cur_dir = 0
        return "up"
"""