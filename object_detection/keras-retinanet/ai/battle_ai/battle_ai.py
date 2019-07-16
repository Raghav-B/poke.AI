import cv2
import numpy as np
from mss import mss
import pyautogui as pag
from PIL import Image
import sys

class battle_ai:
    states = ["entered_battle", "intro_anim", "action_select", "ongoing_turn", "win", "lose"]
    cur_state = "entered_battle"

    z_press_img = None
    action_select_img = None

    pokemon_hp = 141
    opponent_hp = 0

    def __init__(self):
        z_press_img = cv2.imread("z_press.png")
        z_press_img = Image.fromarray(z_press_img)

        action_select_img = cv2.imread("action_select.png")
        action_select_img = Image.fromarray(action_select_img)



    def main_battle_loop(self, ctrl, sct, game_window_size):
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

        while True:
            cv2.imshow("Battle Screen", frame)
            print("Current state: " + str(self.cur_state))
            cv2.waitKey(1)

            if (self.cur_state == "entered_battle"):
                frame_pil = Image.fromarray(frame)
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "intro_anim"
            
            elif (self.cur_state == "intro_anim"):
                frame_pil = Image.fromarray(frame)
                detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "action_select"
            
            elif (self.cur_state == "action_select"):
                # Reset selector position to "FIGHT"
                ctrl.move_up()
                ctrl.move_left()
                
                # If fight is selected
                ctrl.interact()
                # Reset selector position to first move
                ctrl.move_up()
                ctrl.move_left()

                # First move
                ctrl.interact()

                # Second move
                #ctrl.move_right()
                #ctrl.interact()

                # Third move
                #ctrl.move_down()
                #ctrl.interact()

                # Fourth move
                #ctrl.move_down()
                #ctrl.move_right()
                #ctrl.interact()

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
                            #cv2.drawContours(input_img, [cnt], 0, (0, 0, 255), 1)

                        # Opponenet's HP
                        elif (centroid_x >= 140 and centroid_x <= 310 and centroid_y >= 200 and centroid_y <= 250):
                            leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                            rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                            self.opponent_hp = 141 - (rightmost[0] - leftmost[0])
                            #cv2.drawContours(input_img, [cnt], 0, (0, 0, 255), 1)
                
                if (self.pokemon_hp < 0):
                    self.pokemon_hp = 0
                if (self.opponent_hp < 0):
                    self.opponent_hp = 0

                print("Pokemon HP: " + str(self.pokemon_hp))
                print("Opponent HP: " + str(self.opponent_hp))

                # If current opponent pokemon has been beaten
                if (self.opponent_hp <= 0):
                    self.cur_state = "enemy_fainted"
                    # Check for red arrow ..fainted
                    # Press Z
                    # Check for red arrow ..exp points
                    # Press Z
                    # If red arrow, then won
                    # If fight, then next pokemon
                    pass

                # If my pokemon has been beaten
                elif (self.pokemon_hp <= 0):
                    self.cur_state = "lose"
                    # Check for red arrow
                    # Press Z
                    # Whited out...
                    # Press Z
                    pass

                # Only reaches here when enemy hasn't been beaten yet
                # Action select screen once again
                else:
                    self.cur_state = "ongoing_turn"

            elif (self.cur_state == "ongoing_turn"):
                frame_pil = Image.fromarray(frame)
                detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "action_select"
            
            elif (self.cur_state == "enemy_fainted"):
                frame_pil = Image.fromarray(frame)
                # Check for "enemy fainted!" red arrow
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "gained_exp"
                    ctrl.interact()
            
            elif (self.cur_state == "gained_exp"):
                frame_pil = Image.fromarray(frame)
                # Check for "enemy fainted!" red arrow
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "defeated_opponent"
                    ctrl.interact()
                    continue # If this happens, we do not have to check for action select
                
                next_pokemon_detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                if (next_pokemon_detected != None):
                    self.opponent_hp = 141
                    self.cur_state = "action_select"
            
            elif (self.cur_state == "defeated_opponent"):
                frame_pil = Image.fromarray(frame)
                # Check for "enemy fainted!" red arrow
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    self.cur_state = "defeat_dialogue"
                    ctrl.interact()
            
            elif (self.cur_state == "defeat_dialogue"):
                # Actually this doesn't involve checking for a red arrow.
                #frame_pil = Image.fromarray(frame)
                # Check for "enemy fainted!" red arrow
                #detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                #if (detected != None):
                time.sleep(1)
                self.cur_state = "get_money"
                ctrl.interact()

            elif (self.cur_state == "get_money"):
                frame_pil = Image.fromarray(frame)
                # Check for "enemy fainted!" red arrow
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
                if (detected != None):
                    ctrl.interact()
                    time.sleep(2)
                    return # This is when you quit to get to normal overworld
            
            elif (self.cur_state == "lose"):
                print("oh no, you've lost!")
                return
                
                
                    
