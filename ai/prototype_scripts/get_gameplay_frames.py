import cv2
import numpy as np
from mss import mss
import pyautogui as pag

# Finding game window using included .png
game_window_size = {"top": 0, "left": 0, "width": 720, "height": 480}
game_window_size["left"], game_window_size["top"], temp1, temp2 = pag.locateOnScreen("../find_game_window_windows.png")
game_window_size["top"] += 61

# Bring game window into focus so controls can be sent
pag.moveTo(game_window_size["left"] + 100, game_window_size["top"] + 100)
pag.click() # Click on window to focus

sct = mss()

frame_num = 0
while True:
    frame = np.array(sct.grab(game_window_size))
    frame = frame[:, :, :3] # Splicing off alpha channel

    # Making input a square by padding
    game_width = game_window_size["width"]
    game_height = game_window_size["height"]
    padding = 0
    if game_height < game_width:
        padding = int((game_width - game_height) / 2)
        frame = cv2.copyMakeBorder(frame, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
    elif game_height > game_width:
        padding = int((game_width - game_width) / 2)
        frame = cv2.copyMakeBorder(frame, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))

    cv2.imwrite("gameplay_frames/" + str(frame_num) + ".jpg", frame)
    frame_num += 1