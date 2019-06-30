import pyautogui as pag
import time
import random

corner_x = 0
corner_y = 0
key_hold_time = 0.25

# Storing character facing direction.
cur_dir = 0 #0=down, 1=left, 2=up, 3=right

class controller:
    def __init__(self, x, y):
        pag.FAILSAFE = True
        pag.PAUSE = 0
        self.corner_x = x
        self.corner_y = y
        # Bring game window into focus so controls can be sent
        pag.moveTo(self.corner_x + 100, self.corner_y + 100)
        pag.click()
    
    def move_up(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("up")
        pag.keyUp("up")
        return "up"

    def move_right(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("right")
        pag.keyUp("right")
        return "right"

    def move_down(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("down")
        pag.keyUp("down")
        return "down"

    def move_left(self):
        start_time = time.time()
        while time.time() - start_time < key_hold_time:
            pag.keyDown("left")
        pag.keyUp("left")
        return "left"

    def interact(self):
        pag.press("x")
        return "x"

    def random_movement(self):
        action = random.randint(4, 4)
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

    def dummy(self):
        pag.press("")