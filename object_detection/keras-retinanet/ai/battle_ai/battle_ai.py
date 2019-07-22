import cv2
import numpy as np
from mss import mss
import pyautogui as pag
from PIL import Image
import time

# Shift + F1 saves state
# F1 Loads the same state
# Use this when battle has been lost.

class battle_ai:
    states = ["entered_battle", "intro_anim", "action_select", "ongoing_turn", "win", "lose"]
    cur_state = "entered_battle"

    z_press_img = None
    action_select_img = None

    pokemon_hp = 141
    opponent_hp = 141

    def __init__(self):
        self.z_press_img = cv2.imread("D:/App Development/pokemon_ai/object_detection/keras-retinanet/ai/battle_ai/z_press.png")
        self.z_press_img = Image.fromarray(self.z_press_img)
        #cv2.imshow("hm", z_press_img)

        self.action_select_img = cv2.imread("D:/App Development/pokemon_ai/object_detection/keras-retinanet/ai/battle_ai/action_select.png")
        self.action_select_img = Image.fromarray(self.action_select_img)

    def main_battle_loop(self, ctrl, sct, game_window_size):
        while True:
            # Getting game screen as input
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

            cv2.imshow("Screen", frame)
            print("Current state: " + str(self.cur_state))
            cv2.waitKey(1)

            # This state introduces the enemy, for example Younger Allen would like to battle!
            # or Wild Slugma appeared! Need to press Z to continue
            if (self.cur_state == "entered_battle"):
                frame_pil = Image.fromarray(frame)
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                print(detected)
                if (detected != None):
                    self.cur_state = "intro_anim"
                    time.sleep(0.1)
                    ctrl.interact()
            
            # This state shows our pokemon (and in a trainer battle, the opponent's pokemon being sent out)
            # This leads us to the action select screen.
            elif (self.cur_state == "intro_anim"):
                frame_pil = Image.fromarray(frame)
                detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "action_select"
            
            # This is the action select screen. This is the part that the model will really control.
            elif (self.cur_state == "action_select"):
                time.sleep(0.1)

                # Reset selector position to "FIGHT"
                ctrl.move_up()
                ctrl.move_left()
                
                # If fight is selected
                ctrl.interact()
                # Reset selector position to first move
                ctrl.move_up()
                ctrl.move_left()

                ### First move ###
                ctrl.interact()

                ### Second move ###
                #ctrl.move_right()
                #ctrl.interact()

                ### Third move ###
                #ctrl.move_down()
                #ctrl.interact()

                ### Fourth move ###
                #ctrl.move_down()
                #ctrl.move_right()
                #ctrl.interact()

                self.cur_state = "ongoing_turn"

            # This state is when we've selected an attack and both pokemon are performing their individual attacks
            elif (self.cur_state == "ongoing_turn"):
                # HP Detection
                black_lower_bound = (87, 0, 0)
                black_upper_bound = (164, 74, 91)
                black_detection_img = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
                black_detection_img = cv2.inRange(black_detection_img, black_lower_bound, black_upper_bound)
                contours, hierarchy = cv2.findContours(black_detection_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                for cnt in contours:
                    if (cv2.contourArea(cnt) > 0):
                        M = cv2.moments(cnt)
                        centroid_x = int(M['m10']/M['m00'])
                        centroid_y = int(M['m01']/M['m00'])

                        # My HP
                        if (centroid_x >= 510 and centroid_x <= 680 and centroid_y >= 370 and centroid_y <= 420):
                            leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                            rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                            self.pokemon_hp = 141 - (rightmost[0] - leftmost[0])

                        # Opponenet's HP
                        elif (centroid_x >= 140 and centroid_x <= 310 and centroid_y >= 200 and centroid_y <= 250):
                            leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                            rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                            self.opponent_hp = 141 - (rightmost[0] - leftmost[0])
                
                if (self.pokemon_hp < 0):
                    self.pokemon_hp = 0
                if (self.opponent_hp < 0):
                    self.opponent_hp = 0

                print("Pokemon HP: " + str(self.pokemon_hp))
                print("Opponent HP: " + str(self.opponent_hp))

                print("Finding next state...")
                # If current opponent pokemon or my pokemon has been beaten
                if (self.opponent_hp <= 0 or self.pokemon_hp <= 0):
                    self.cur_state = "battle_ended"
                # Only reaches here when enemy hasn't been beaten yet
                # Action select screen once again
                else:                
                    frame_pil = Image.fromarray(frame)
                    detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                    if (detected != None):
                        self.cur_state = "action_select"
            
            elif (self.cur_state == "battle_ended"):                
                # HP Detection
                end_lower_bound = (0, 0, 0)
                end_upper_bound = (0, 0, 0)
                end_detection_img = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
                end_detection_img = cv2.inRange(end_detection_img, end_lower_bound, end_upper_bound)
                contours, hierarchy = cv2.findContours(end_detection_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                has_battle_ended = False
                for cnt in contours:
                    if (cv2.contourArea(cnt) > 100000):
                        print("battle has actually ended")
                        has_battle_ended = True
                        self.cur_state = "entered_battle"
                        self.opponent_hp = 141
                        return
                
                if (has_battle_ended == False):
                    frame_pil = Image.fromarray(frame)

                    # This happens when the trainer is sending out another pokemon
                    action_select_detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                    if (action_select_detected != None):
                        self.cur_state = "action_select"
                        self.opponent_hp = 141
                        continue
                    
                    # This is to handle any required key presses due to levelling or other stuff
                    time.sleep(0.1)
                    ctrl.interact()