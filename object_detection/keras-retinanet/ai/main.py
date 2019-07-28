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
#from fastgrab._linux_x11 import screenshot
from mss import mss
import pyautogui as pag
import time
import sys

# Custom imports
from mapper import live_map
from auto_controller import controller
from battle_ai.battle_ai import battle_ai


# Dummy function, does nothing
def nothing(x):
    pass


# Some keras/tensorflow related stuff, even I'm not entirely sure what it does exactly
def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)


# Initializes model for detection, mapper, controller and finds game window
def initialise(game_window_size, model_path):
    keras.backend.tensorflow_backend.set_session(get_session())
    detection_model = models.load_model(model_path, backbone_name='resnet101')

    # Load battle AI model here as well.
    battle_model = Sequential()
    battle_model.add(Dense(24, input_dim=2, activation='relu')) # 2 is the number of inputs into our model
    battle_model.add(Dense(24, activation='relu'))
    battle_model.add(Dense(4, activation='linear')) # 4 is the number of actions we can choose from
    battle_model.compile(loss='mse', optimizer=Adam(lr=0.001)) # learning rate
    # Initialising battle ai with battle_model
    bat_ai = battle_ai(battle_model)

    # Setting up windows
    cv2.namedWindow("Map", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("Map", game_window_size["width"], game_window_size["height"])
    cv2.moveWindow("Map", 1100, 850)
    cv2.namedWindow("Screen")
    cv2.moveWindow("Screen", 1100, 0)
    cv2.createTrackbar("ScoreThresh", "Screen", 70, 99, nothing)

    # Finding game window using included .png
    game_window_size["left"], game_window_size["top"], temp1, temp2 = pag.locateOnScreen("find_game_window_windows.png")

    # Adding a 76 pixel offset to the y coordinate since the function above returns the x,y
    # coordinates of the menu bar - we want the coords of the gameplay below this bar
    # Change this offset to 20 if you are running on ubuntu and are using find_game_window_ubuntu.png
    game_window_size["top"] += 76

    # Setup controller
    ctrl = controller(game_window_size["left"], game_window_size["top"])

    # Initialise screen capturer
    sct = mss()
    
    # Get padding for converting 720:480 aspect ratio to 1:1
    temp3, padding = get_screen(sct, game_window_size)
    
    # Initialising mapper object with retroarch pid and memory addresses to watch for player's
    # x and y coordinates
    mp = live_map(game_window_size["width"], game_window_size["height"], padding)

    return game_window_size, sct, ctrl, bat_ai, detection_model, mp


# Gets single frame of gameplay as a numpy array on which object inference will be later ran
def get_screen(sct, game_window_size):
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
    
    return frame, padding


# Runs inference on a single input frame and returns detected bounding boxes
def run_detection(frame, detection_model, labels_to_names, mp):
    # Get trackbar value for confidence score threshold
    score_thresh = cv2.getTrackbarPos("ScoreThresh", "Screen") 

    # Process image and run inference
    image = preprocess_image(frame) # Retinanet specific preprocessing
    image, scale = resize_image(image, min_side = 400) # This model was trained with 400p images
    boxes, scores, labels = detection_model.predict_on_batch(np.expand_dims(image, axis=0)) # Run inference
    boxes /= scale # Ensures bounding boxes are of the correct scale

    has_detections = False

    # Visualize detections from inferencing
    predictions_for_map = []
    for box, score, label in zip(boxes[0], scores[0], labels[0]):
        # We can break here because the bounding boxes are in descending order in terms of confidence
        if score < (score_thresh / 100):
            break

        has_detections = True

        # Drawing labels and bounding boxes on input frame
        color = label_color(label)
        b = box.astype(int)
        draw_box(frame, b, color=color)
        caption = "{} {:.2f}".format(labels_to_names[label], score)
        draw_caption(frame, b, caption)

        # Appending to output array
        predictions_for_map.append((label, box))
    
    # Show output frame with detections
    cv2.imshow("Screen", frame)
    status = "ok"
    # Press 'q' to quit safely
    if cv2.waitKey(1) == ord('q'):
        status = "quit"

    return status, has_detections, predictions_for_map, False

# Main function
if __name__ == "__main__":
    # Setup variables here
    game_window_size = {"top": 0, "left": 0, "width": 720, "height": 480}
    model_path = "../inference_graphs/400p/resnet101_csv_13.h5" # Model to be used for detection
    labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"} # Labels to draw

    # Initialising model, window, controller, and mapper
    game_window_size, sct, ctrl, bat_ai, detection_model, mp = initialise(game_window_size, model_path)

    is_init_frame = True
    has_detections = False
    predictions_for_map = []
    temp_bool = None
    key_pressed = None
    key_to_press = None
    map_grid = np.full((2, 2), 255, dtype=np.uint8)
    four_frame_count = 0

    keys = ["up", "right", "down", "left"]

    # Use to set pre-defined actions to send to controller (default is random)
    actions = []
    #actions = [0,0,1,1,2,2,3,3]
    #actions = [2,2,0,0]
    action_index = -1 # Initialise this from -1
    
    # It takes about 5 frames for our player characte to perform a movement in any direction. Thus,
    # it makes no sense to update the map during each of these frames, especially because the character
    # will be stuck between two tiles in some frames and this will throw off our mapping algorithm
    while True:  
        # 0th frame handles key presses
        if (four_frame_count == 0):
            #time.sleep(2) # Adjust this to reduce frequency of actions sent by controller

            # Initial startup frame to put detection and key presses in sync
            if (is_init_frame == True):
                key_pressed = None
                frame, temp = get_screen(sct, game_window_size)
                status, has_detections, predictions_for_map, temp_init = run_detection(frame, detection_model, labels_to_names, mp)
                map_grid, collision_type = mp.draw_map(key_pressed, predictions_for_map)
                # No collision handler here since it is literally impossible to collide on the first frame

                four_frame_count += 1
                cv2.imshow("Map", map_grid[:,:,:3])
                actions = mp.get_movelist()

            # All other 0 frames that are not the initial frame
            else:
                # Used to iterate through pre-defined actions and break once actions have ended
                action_index += 1
                if (action_index >= len(actions)):
                    actions = mp.get_movelist()
                    action_index = 0
                
                print("Key pressed: " + keys[actions[action_index]])            
                key_pressed = ctrl.perform_movement(action=actions[action_index])
                four_frame_count += 1

        # All other frames just deal with normal inferencing for nicer visualization purposes, but this does
        # nothing to affect out mapping algorithm
        elif (four_frame_count < 4):
            frame, temp = get_screen(sct, game_window_size)
            status, has_detections, predictions_for_map, temp_bool = run_detection(frame, detection_model, labels_to_names, mp)
            four_frame_count += 1

            if (status == "quit"):
                break

        # Last frame is when the new detections are properly taken from the inferencing, and are used as inputs in the
        # mapping algorithm
        elif (four_frame_count == 4):
            frame, temp = get_screen(sct, game_window_size)
            status, has_detections, predictions_for_map, temp_bool = run_detection(frame, detection_model, labels_to_names, mp)

            print("Map drawing frame")

            # Draw map in window
            # Take note that there is a one frame delay because of something in OpenCV itself. If you print
            # the map_grid, you'll see that the mapping is actually performed realtime
            map_grid, collision_type = mp.draw_map(key_pressed, predictions_for_map)
            cv2.imshow("Map", map_grid[:,:,:3])
            print(collision_type)
            print("")

            if (collision_type == "battle_collision_post" or collision_type == "battle_collision_pre"):
                if (collision_type == "battle_collision_pre"):
                    action_index -= 1
                    action_index %= len(actions) # Ensuring that any negative values are cycled back to positive

                while (has_detections == True):
                    frame, temp = get_screen(sct, game_window_size)
                    status, has_detections, predictions_for_map, temp_bool = run_detection(frame, detection_model, labels_to_names, mp)
                    
                    # Spam Z until battle has properly started.
                    ctrl.interact()

                # Start battle ai
                battle_status = bat_ai.main_battle_loop(ctrl, sct, game_window_size)
                # Reset battle AI
                if (battle_status == "reset"):
                    action_index = -1
                    ctrl.reload_state()

                    bat_ai.pokemon_hp = 141 # Reset back to default
                    temp3, padding = get_screen(sct, game_window_size)
                    mp = live_map(game_window_size["width"], game_window_size["height"], padding)
                    is_init_frame = True
                    four_frame_count = 0
                    actions = []
                    continue

                # After battle ai has completed, returning back to normal movement
                while True:
                    frame, temp = get_screen(sct, game_window_size)
                    temp1, has_detections, temp2, temp3 = run_detection(frame, detection_model, labels_to_names, mp)
                    if (has_detections == True):
                        time.sleep(2)
                        break

            else:
                # Check here if the latest frontier is now a building or another object. 
                # If it is, search for another frontier.
                if (not (np.array_equal(map_grid[mp.pf.next_frontier[2]][mp.pf.next_frontier[1]][:3], [0, 0, 0]) or \
                    np.array_equal(map_grid[mp.pf.next_frontier[2]][mp.pf.next_frontier[1]][:3], [255, 255, 255]))):
                    print("Frontier obstructed, switching to new frontier...")
                    actions = mp.get_movelist()
                    action_index = -1

                # Change actions to newly calculated path if a collision occurs
                if (collision_type == "normal_collision"):
                    actions = mp.pf.frontier_path_collision_handler(map_grid, \
                        (mp.map_offset_x - mp.map_min_offset_x), \
                        (mp.map_offset_y - mp.map_min_offset_y))
                    if (actions == False): # If we have experienced 5 consecutive collisions
                        # Find a new frontier to go towards
                        actions = mp.get_movelist()
                    action_index = -1 # Either way we reset the index

            # Reset 5 frame cycle
            four_frame_count = 0
        
            if (status == "quit"):
                break

        # Init frame is over, change flag accordingly
        if (is_init_frame == True):
            is_init_frame = temp_bool
    
    # Clean running processes and close program cleanly
    cv2.destroyAllWindows()
    sys.exit()