import pyautogui as pag
import time
import random

# Global variables, these should be inside the class below but I'm too
# lazy to type self. everywhere
corner_x = 0 # x coordinate of top left corner of gameplay
corner_y = 0 # y coordinate
key_hold_time = 1 # If this is too short, character won't move, will instead turn
# Storing character facing direction. CURRENTLY NOT IN USE
cur_dir = 0 # 0=up, 1=right, 2=down, 3=left

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
        pag.keyUp("up")
        cur_dir = 0
        return "up"


    def move_right(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("right")
        pag.keyUp("right")
        cur_dir = 1
        return "right"


    def move_down(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("down")
        pag.keyUp("down")
        cur_dir = 2
        return "down"


    def move_left(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("left")
        pag.keyUp("left")
        cur_dir = 3
        return "left"


    def interact(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("z") # Change to "x" on ubuntu
        pag.keyUp("z") # Change to "x" on ubuntu
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
        pag.keyUp("up")
        cur_dir = 0
        return "up"