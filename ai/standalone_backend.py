import keras

# Imports for DQNN
from keras.models import Sequential
from keras.layers import Dense
from keras.optimizers import Adam

from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color
import tensorflow as tf

import cv2
import numpy as np
np.set_printoptions(linewidth=500) # For map debugging (view map array in terminal)
from mss import mss
import pyautogui as pag
import time
import sys

# Custom imports
from mapper import live_map
from auto_controller import backend_controller as controller
from battle_ai.battle_ai import battle_ai

import threading

class poke_ai:
    def __init__(self, model_path, labels_to_names, game_window_size):
        self.game_window_size = game_window_size
        self.model_path = model_path
        self.labels_to_names = labels_to_names

        keras.backend.tensorflow_backend.set_session(self.get_session())
        self.detection_model = models.load_model(self.model_path, backbone_name="resnet50")

        # Load battle AI model here as well.
        self.battle_model = Sequential()
        self.battle_model.add(Dense(24, input_dim=2, activation='relu')) # 2 is the number of inputs into our model
        self.battle_model.add(Dense(24, activation='relu'))
        self.battle_model.add(Dense(4, activation='linear')) # 4 is the number of actions we can choose from
        self.battle_model.compile(loss='mse', optimizer=Adam(lr=0.001)) # learning rate
        # Initialising battle ai with battle_model
        self.bat_ai = battle_ai(self.battle_model)

        # Finding game window using included .png
        self.game_window_size["left"], self.game_window_size["top"], temp1, temp2 = pag.locateOnScreen("find_game_window_windows.png", confidence=0.8)
        # Adding a 76 pixel offset to the y coordinate since the function above returns the x,y
        # coordinates of the menu bar - we want the coords of the gameplay below this bar
        # Change this offset to 20 if you are running on ubuntu and are using find_game_window_ubuntu.png
        self.game_window_size["top"] += 76

        # Initialise screen capturer
        self.sct = mss()
        # Get padding for converting 720:480 aspect ratio to 1:1
        temp3, padding = self.get_screen()

        # Setup controller
        self.ctrl = controller()
        temp_init_ram_vals = self.ctrl.get_ram_vals()
        # Initialising mapper object with retroarch pid and memory addresses to watch for player's
        # x and y coordinates
        self.mp = live_map(self.game_window_size["width"], self.game_window_size["height"], padding, temp_init_ram_vals)

        # Variables used for running each step of AI
        self.step_count = 0
        self.is_init_step = True
        self.has_detections = False
        self.predictions_for_map = []
        self.ram_vals = self.mp.prev_ram

        self.key_pressed = None
        self.keys = ["up", "right", "down", "left"]
        self.actions = []
        self.action_index = -1
        self.collision_type = "no_collision"

        self.in_battle = False
        self.map_grid = np.full((2, 2), 255, dtype=np.uint8)

        # Variables to keep track of mapper history
        self.mapper_history_list = []
        self.history_output = None

    # Dummy function, does nothing
    def nothing(self, x):
        pass

    # Some keras/tensorflow related stuff, even I'm not entirely sure what it does exactly
    def get_session(self):
        config = tf.ConfigProto()
        config.gpu_options.allow_growth = True
        return tf.Session(config=config)

    # Gets single frame of gameplay as a numpy array on which object inference will be later ran
    def get_screen(self):
        # Getting game screen as input
        frame = np.array(self.sct.grab(self.game_window_size))
        frame = frame[:, :, :3] # Splicing off alpha channel

        # Making input a square by padding
        game_width = self.game_window_size["width"]
        game_height = self.game_window_size["height"]
        padding = 0
        if game_height < game_width:
            padding = int((game_width - game_height) / 2)
            frame = cv2.copyMakeBorder(frame, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
        elif game_height > game_width:
            padding = int((game_height - game_width) / 2)
            frame = cv2.copyMakeBorder(frame, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))
        
        return frame, padding

    # Runs inference on a single input frame and returns detected bounding boxes
    def run_detection(self, frame):
        # Process image and run inference
        image = preprocess_image(frame) # Retinanet specific preprocessing
        image, scale = resize_image(image, min_side = 400) # This model was trained with 400p images
        boxes, scores, labels = self.detection_model.predict_on_batch(np.expand_dims(image, axis=0)) # Run inference
        boxes /= scale # Ensures bounding boxes are of the correct scale

        self.has_detections = False

        # Visualize detections from inferencing
        self.predictions_for_map = []
        for box, score, label in zip(boxes[0], scores[0], labels[0]):
            # We can break here because the bounding boxes are in descending order in terms of confidence
            if score < (85 / 100):
                break

            if (label == 7):
                continue

            self.has_detections = True

            # Drawing labels and bounding boxes on input frame
            color = label_color(label)
            b = box.astype(int)
            draw_box(frame, b, color=color)
            caption = "{} {:.2f}".format(self.labels_to_names[label], score)
            draw_caption(frame, b, caption)

            # Appending to output array
            self.predictions_for_map.append((label, box))

        return frame, False

    def run_step(self):
        temp_bool = None
        frame = None
        
        if (self.in_battle == False):
            
            
            if (self.step_count == 0):
                if (self.is_init_step == True):
                    self.key_pressed = None
                    frame, temp = self.get_screen()
                    frame, temp_init = self.run_detection(frame)
                    self.map_grid, self.collision_type = self.mp.draw_map(self.key_pressed, self.predictions_for_map, self.ram_vals)
                    # No collision handler here since it is literally impossible to collide on the first frame

                    self.step_count += 1
                    self.actions = self.mp.get_movelist()
                    temp_obj = mapping_history_list_obj(f"Found next frontier at {self.mp.pf.next_frontier[1:]}", \
                        frame, self.map_grid)
                    self.mapper_history_list.append(temp_obj)

                # All other 0 frames that are not the initial frame
                else:
                    frame, temp = self.get_screen()
                    frame, temp_init = self.run_detection(frame)
                    
                    # Used to iterate through pre-defined actions and break once actions have ended
                    self.action_index += 1
                    if (self.action_index >= len(self.actions)):
                        self.action_index = 0
                        self.actions = self.mp.get_movelist()
                        temp_obj = mapping_history_list_obj(f"Reached frontier at {self.mp.pf.next_frontier[1:]}", \
                            frame, self.map_grid)
                        self.mapper_history_list.append(temp_obj)
                        temp_obj = mapping_history_list_obj(f"Found next frontier at {self.mp.pf.next_frontier[1:]}", \
                            frame, self.map_grid)
                        self.mapper_history_list.append(temp_obj)
                    
                    print("Key pressed: " + self.keys[self.actions[self.action_index]])            
                    self.key_pressed, self.ram_vals = self.ctrl.perform_movement(action=self.actions[self.action_index])
                    self.step_count += 1



            # All other frames just deal with normal inferencing for nicer visualization purposes, but this does
            # nothing to affect out mapping algorithm
            elif (self.step_count < 4):
                frame, temp = self.get_screen()
                frame, temp_bool = self.run_detection(frame)
                self.step_count += 1



            # Last frame is when the new detections are properly taken from the inferencing, and are used as inputs in the
            # mapping algorithm
            elif (self.step_count == 4):
                frame, temp = self.get_screen()
                frame, temp_bool = self.run_detection(frame)

                # Draw map in window
                # Take note that there is a one frame delay because of something in OpenCV itself. If you print
                # the map_grid, you'll see that the mapping is actually performed realtime
                self.map_grid, self.collision_type = self.mp.draw_map(self.key_pressed, self.predictions_for_map, self.ram_vals)
                print(self.collision_type)
                print("")

                if (self.key_pressed == None):
                    temp_obj = mapping_history_list_obj(f"Initialising controller", \
                        frame, self.map_grid)
                    self.mapper_history_list.append(temp_obj)
                else:
                    temp_obj = mapping_history_list_obj(f"Moved {self.keys[self.key_pressed]}", \
                        frame, self.map_grid)
                    self.mapper_history_list.append(temp_obj)

                if (self.collision_type == "battle_collision_post" or self.collision_type == "battle_collision_pre"):
                    if (self.collision_type == "battle_collision_pre"):
                        self.action_index -= 1
                        self.action_index %= len(self.actions) # Ensuring that any negative values are cycled back to positive

                    while (self.has_detections == True):
                        frame, temp = self.get_screen()
                        frame, temp_bool = self.run_detection(frame)
                        
                        # Spam Z until battle has properly started.
                        self.ctrl.interact()
                    self.in_battle = True

                else:
                    # Check here if the latest frontier is now a building or another object. 
                    # If it is, search for another frontier.
                    if (not (np.array_equal(self.map_grid[self.mp.pf.next_frontier[2]][self.mp.pf.next_frontier[1]][:3], [0, 0, 0]) or \
                        np.array_equal(self.map_grid[self.mp.pf.next_frontier[2]][self.mp.pf.next_frontier[1]][:3], [255, 255, 255]))):
                        print("Frontier obstructed, switching to new frontier...")
                        temp_obj = mapping_history_list_obj(f"Frontier at {self.mp.pf.next_frontier[1:]} obstructued", \
                            frame, self.map_grid)
                        self.mapper_history_list.append(temp_obj)
                        self.action_index = -1
                        self.actions = self.mp.get_movelist()
                        temp_obj = mapping_history_list_obj(f"Found next frontier at {self.mp.pf.next_frontier[1:]}", \
                            frame, self.map_grid)
                        self.mapper_history_list.append(temp_obj)

                    # Change actions to newly calculated path if a collision occurs
                    if (self.collision_type == "normal_collision"):
                        temp_obj = mapping_history_list_obj(f"Collision occured, finding new path to frontier", \
                            frame, self.map_grid)
                        self.mapper_history_list.append(temp_obj)
                        self.actions = self.mp.pf.frontier_path_collision_handler(self.map_grid, \
                            (self.mp.map_offset_x - self.mp.map_min_offset_x), \
                            (self.mp.map_offset_y - self.mp.map_min_offset_y))
                        if (self.actions == False): # If we have experienced 5 consecutive collisions
                            # Find a new frontier to go towards
                            self.actions = self.mp.get_movelist()
                            temp_obj = mapping_history_list_obj(f"Found next frontier at {self.mp.pf.next_frontier[1:]}", \
                                frame, self.map_grid)
                            self.mapper_history_list.append(temp_obj)
                        self.action_index = -1 # Either way we reset the index

                # Reset 5 frame cycle
                self.step_count = 0

            # Init frame is over, change flag accordingly
            if (self.is_init_step == True):
                self.is_init_step = temp_bool
        
        else:
            # Start battle ai
            frame, battle_status = self.bat_ai.main_battle_loop(self.ctrl, self.sct, self.game_window_size)
            # Reset battle AI
            if (battle_status == "reset"):
                self.action_index = -1
                self.ctrl.reload_state()

                self.bat_ai.pokemon_hp = 141 # Reset back to default
                temp3, padding = self.get_screen()
                self.mp = live_map(self.game_window_size["width"], self.game_window_size["height"], padding, self.ram_vals)
                self.is_init_step = True
                self.step_count = 0
                self.actions = []
                self.in_battle = False
                return frame, self.map_grid

            elif (battle_status == "continue"):
                self.in_battle = True
                return frame, self.map_grid
            
            else:
                self.in_battle = False

            # After battle ai has completed, returning back to normal movement
            while True:
                frame, temp = self.get_screen()
                frame, temp3 = self.run_detection(frame)
                if (self.has_detections == True):
                    time.sleep(0.5)
                    break

        # Drawing centroid on map
        ret_map_grid = self.map_grid.copy()
        ret_map_grid[self.mp.pf.next_frontier[2]][self.mp.pf.next_frontier[1]][:3] = [234, 0, 255]
        return frame, ret_map_grid

    def show_windows(self, frame, map_grid):
        cv2.imshow("Screen", frame)
        cv2.imshow("Map", map_grid[:,:,:3])

class mapping_history_list_obj:
    def __init__(self, text, detection_img, map_img):
        self.text = text
        self.detection_img = detection_img
        self.map_img = map_img

# Main function
if __name__ == "__main__":
    # Setup variables here
    game_window_size = {"top": 0, "left": 0, "width": 720, "height": 480}
    #model_path = "../object_detection/keras-retinanet/inference_graphs/map_detector.h5" # Model to be used for detection
    #labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"} # Labels to draw

    model_path = "../object_detection/keras-retinanet/inference_graphs/resnet50_csv_13.h5" # Model to be used for detection
    labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit", 6: "wall", 7:"grass"} # Labels to draw

    # Setting up windows
    cv2.namedWindow("Map", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("Map", game_window_size["width"], game_window_size["height"])
    cv2.moveWindow("Map", 750, 850)
    cv2.namedWindow("Screen")
    cv2.moveWindow("Screen", 750, 0)
    
    my_poke_ai = poke_ai(model_path, labels_to_names, game_window_size)

    while True:  
        frame, map_grid = my_poke_ai.run_step()
        my_poke_ai.show_windows(frame, map_grid)
        
        key = cv2.waitKey(1)
        if (key == ord('q')):
            break

    # Clean running processes and close program cleanly
    cv2.destroyAllWindows()
    sys.exit()