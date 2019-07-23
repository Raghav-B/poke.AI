import cv2
import random
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

    # DQNN Variables
    battle_model = None
    battle_data = [] # Contains an unsorted history of all state and action pairs so far
    gamma = 0.95
    epsilon = 1.0
    epsilon_min = 0.01
    epsilon_decay = 0.995
    train_batch_size = 32

    num_episodes_completed = 0
    
    # Keeping track of state related variables
    init_state = None
    next_state = None
    move_index = None


    def __init__(self, battle_model):
        self.z_press_img = cv2.imread("D:/App Development/pokemon_ai/object_detection/keras-retinanet/ai/battle_ai/z_press.png")
        self.z_press_img = Image.fromarray(self.z_press_img)
        #cv2.imshow("hm", z_press_img)

        self.action_select_img = cv2.imread("D:/App Development/pokemon_ai/object_detection/keras-retinanet/ai/battle_ai/action_select.png")
        self.action_select_img = Image.fromarray(self.action_select_img)

        self.battle_model = battle_model

    def update_hps(self, frame):
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


    def action_performer(self, ctrl):
        # Reset selector position to "FIGHT"
        time.sleep(0.1)
        ctrl.move_up()
        ctrl.move_left()
        # If fight is selected (for now we are only sticking with selecting fight)
        ctrl.interact()

        print("Move selected: " + str(self.move_index))

        # Reset move selector to 0 (top_left move)
        time.sleep(0.1)
        ctrl.move_up()
        ctrl.move_left()

        if (self.move_index == 0): # First move and so on
            ctrl.interact()
        elif (self.move_index == 1):
            ctrl.move_right()
            ctrl.interact()
        elif (self.move_index == 2):
            ctrl.move_down()
            ctrl.interact()
        else: # 4th, last move
            ctrl.move_down()
            ctrl.move_right()
            ctrl.interact()

    
    def do_training_step(self):
        print("Performing training...")

        # Get random sample of training data of batch_size 32
        training_batch = random.sample(self.battle_data, self.train_batch_size)
        train_state_arr = []
        train_target_arr = []

        for init_state, action, reward, next_state, done in training_batch:
            target_reward = reward

            if (done == False):
                # If haven't reached terminal state, the target_reward value is based on the predicted
                # expected reward from next_state
                target_reward = (reward + self.gamma * np.amax(self.battle_model.predict(next_state)[0]))
            
            reward_prediction = self.battle_model.predict(init_state)
            reward_prediction[0][action] = target_reward

            train_state_arr.append(init_state[0])
            train_target_arr.append(reward_prediction[0])
        
        # Actual keras training function
        history = self.battle_model.fit(np.array(train_state_arr), np.array(train_target_arr), epochs=1, verbose=0)
        # Keeping track of our loss
        loss = history.history["loss"][0]
        # Reduce randomness (exploration vs exploitation thing)
        if (self.epsilon > self.epsilon_min):
            self.epsilon *= self.epsilon_decay
        return loss


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
            #print("Current state: " + str(self.cur_state))
            cv2.waitKey(1)


            # This state introduces the enemy, for example Younger Allen would like to battle!
            # or Wild Slugma appeared! Need to press Z to continue
            if (self.cur_state == "entered_battle"):
                frame_pil = Image.fromarray(frame)
                detected = pag.locate(self.z_press_img, frame_pil, grayscale=False, confidence=0.9)
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
                self.init_state = np.zeros((1, 2))
                self.update_hps(frame)
                self.init_state[0][0] = self.pokemon_hp
                self.init_state[0][1] = self.opponent_hp
                
                self.move_index = 0
                if (np.random.rand() <= self.epsilon):
                    print("Exploration (randomness selected)")
                    self.move_index = random.randint(0, 3) #0, 3
                else:
                    print("Exploitation (model-based)")
                    action_predicted_rewards = self.battle_model.predict(self.init_state)
                    self.move_index = np.argmax(action_predicted_rewards[0])
                
                # Performing actual, physical action now
                self.action_performer(ctrl)
                self.cur_state = "ongoing_turn"

            # This state is when we've selected an attack and both pokemon are performing their individual attacks
            elif (self.cur_state == "ongoing_turn"):
                # This state will be called multiple times since we don't know how long the pokemons' turns
                # will last. Thus we need to continue updating our stored HP values so they're ready to use once
                # the next state has been detected.
                self.update_hps(frame)
                #print("Pokemon HP: " + str(self.pokemon_hp))
                #print("Opponent HP: " + str(self.opponent_hp))
                #print("Finding next state...")
                #print("")

                # If current opponent pokemon or my pokemon has been beaten
                if (self.opponent_hp <= 0 or self.pokemon_hp <= 0): # Need to handle self-death in a nicer way eventually
                    self.next_state = np.zeros((1, 2))
                    self.next_state[0][0] = self.pokemon_hp
                    self.next_state[0][1] = self.opponent_hp

                    # Performing reward calculation
                    base_reward = 0
                    if (self.opponent_hp <= 0):
                        base_reward = 200
                    elif (self.pokemon_hp <= 0):
                        base_reward = -200
                    reward = (self.init_state[0][1] - self.next_state[0][1]) - \
                        (self.init_state[0][0] - self.next_state[0][0]) + base_reward
                    print("Action reward: " + str(reward))

                    # Adding this state/action pair to our dataset. Last element is True because 1v1 battle
                    # has ended in this conditional
                    self.battle_data.append((self.init_state, self.move_index, reward, self.next_state, True))
                    self.cur_state = "battle_ended"
                
                # Only reaches here when enemy hasn't been beaten yet
                # Action select screen once again
                else:                
                    frame_pil = Image.fromarray(frame)
                    detected = pag.locate(self.action_select_img, frame_pil, grayscale=False, confidence=0.9)
                    if (detected != None):
                        self.next_state = np.zeros((1, 2))
                        self.next_state[0][0] = self.pokemon_hp
                        self.next_state[0][1] = self.opponent_hp
                        
                        # Performing reward calculation for our last used move
                        reward = (self.init_state[0][1] - self.next_state[0][1]) - \
                            (self.init_state[0][0] - self.next_state[0][0])
                        print("Action reward: " + str(reward))

                        # Adding this state/action pair to our dataset. Last element is False because 1v1 battle
                        # is still going on
                        self.battle_data.append((self.init_state, self.move_index, reward, self.next_state, False))
                        self.cur_state = "action_select"
                
                if (self.cur_state == "battle_ended"):
                    self.num_episodes_completed += 1
                    print(f"Episode: {self.num_episodes_completed}, Randomness: {self.epsilon}")
                    print("")
                    # Save the model every 5 episodes
                    if (self.num_episodes_completed % 5 == 0):
                        print("Model saved!")
                        print("")
                        self.battle_model.save_weights(f"battle_ai/models/battle_model_{self.num_episodes_completed}.h5")

                # This conditional basically allows the battle_model to perform its training after every state
                # pair provided that the minimum batch_size in battle_data has been achieved.
                if (self.cur_state == "battle_ended" or self.cur_state == "action_select"):
                    print("Current batch size: " + str(len(self.battle_data)))
                    if (len(self.battle_data) > self.train_batch_size):
                        loss = self.do_training_step()
                        print(f"Episode: {self.num_episodes_completed}, Loss: {loss}")
                        print("")

            elif (self.cur_state == "battle_ended"):                
                # Fade to black detection
                end_lower_bound = (0, 0, 0)
                end_upper_bound = (0, 0, 0)
                end_detection_img = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
                end_detection_img = cv2.inRange(end_detection_img, end_lower_bound, end_upper_bound)
                contours, hierarchy = cv2.findContours(end_detection_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                has_battle_ended = False
                for cnt in contours:
                    if (cv2.contourArea(cnt) > 100000):
                        #print("battle has actually ended")
                        has_battle_ended = True
                        self.cur_state = "entered_battle"
                        self.opponent_hp = 141
                        
                        if (self.pokemon_hp <= 0):
                            return "reset"
                        else:
                            return "end"
                
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
    
