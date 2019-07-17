#4636.jpg
#4787.jpg

import cv2
import numpy as np
from PIL import Image
import pyautogui as pag
import time

test_img = cv2.imread("gameplay_frames/8443.jpg")
#test_img = cv2.cvtColor(test_img, cv2.COLOR_BGR2RGB)
test_img_pil = Image.fromarray(test_img)

red_button = cv2.imread("press_z_battle.png")
red_button_pil = Image.fromarray(red_button)

start_time = time.time()
location = pag.locate(red_button_pil, test_img_pil, grayscale=False, confidence=0.9)
print("Time taken: " + str(time.time() - start_time))

print(location)

while True:
    cv2.imshow("a", test_img)
    cv2.imshow("b", red_button)
    if cv2.waitKey() == ord('q'):
        break
