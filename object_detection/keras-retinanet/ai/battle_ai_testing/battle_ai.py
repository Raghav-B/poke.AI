import cv2
import numpy as np
from mss import mss
import pyautogui as pag
from PIL import Image
from auto_controller import controller
import sys

class battle_ai:
    states = ["entered_battle", "intro_anim", "action_select", "ongoing_turn", "win", "lose"]
    cur_state = "entered_battle"

    z_press_img = None
    action_select_img = None

    def __init__(self):
        z_press_img = cv2.imread("")