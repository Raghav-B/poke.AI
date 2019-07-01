import keras
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color
import tensorflow as tf

import cv2
import numpy as np
np.set_printoptions(linewidth=500)
from fastgrab._linux_x11 import screenshot
import pyautogui as pag
import time
import threading
import sys
import matplotlib.pyplot as plt

# Custom imports
from sort_objects import object_sorter
from mapper import live_map
from auto_controller import controller

def nothing(x):
    pass

def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)

def initialise(game_window, game_width, game_height, model_path):
    keras.backend.tensorflow_backend.set_session(get_session())
    model = models.load_model(model_path, backbone_name='resnet101')

    cv2.namedWindow("Map", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("Map", 720, 480)
    cv2.namedWindow("Screen")
    cv2.moveWindow("Screen", 1000, 0)
    cv2.createTrackbar("ScoreThresh", "Screen", 70, 99, nothing)

    window_x, window_y, temp1, temp2 = pag.locateOnScreen("find_game_window.png")
    # Adding a 20 pixel offset to the y coordinate since our gameplay is slightly lower.
    window_y += 20
    # Setup controller
    ctrl = controller(window_x, window_y)
    # Get padding for square conversion
    temp3, padding = get_screen(game_window, window_x, window_y)
    # Initialising mapper object
    mp = live_map(game_width, game_height, padding, 4681, 0x55c106d0bf5c, 0x55c106d0bf5e)
    # Initialising object sorter

    return ctrl, window_x, window_y, model, mp

def get_screen(game_window, window_x, window_y):
    # Getting game screen as input
    screenshot(window_x, window_y, game_window)
    frame = game_window[:, :, :3]

    # Making input a square
    padding = 0
    if game_height < game_width:
        padding = int((game_width - game_height) / 2)
        frame = cv2.copyMakeBorder(frame, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
        #print(padding)
    elif game_height > game_width:
        padding = int((game_width - game_width) / 2)
        frame = cv2.copyMakeBorder(frame, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))
    
    return frame, padding

def run_detection(frame, model, labels_to_names, mp):
    score_thresh = cv2.getTrackbarPos("ScoreThresh", "Screen")

    # Process image and run inference
    image = preprocess_image(frame)
    image, scale = resize_image(image, min_side = 400)
    boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0))
    boxes /= scale

    # Visualize detections from inferencing
    predictions_for_map = []
    for box, score, label in zip(boxes[0], scores[0], labels[0]):
        if score < (score_thresh / 100):
            break
        
        if (label == 2):
            continue
        predictions_for_map.append((label, box))

        # Draw labels and bounding boxes
        color = label_color(label)
        b = box.astype(int)
        draw_box(frame, b, color=color)
        caption = "{} {:.2f}".format(labels_to_names[label], score)
        draw_caption(frame, b, caption)

    #font = cv2.FONT_HERSHEY_SIMPLEX
    #for point in cur_frame_objects:
        #cv2.putText(frame, str(point[2]), point[0], font, 0.5, (255, 255, 0), 2, cv2.LINE_AA)

    # Show grid lines
    #for i in range(0, 16): # drawing vertical lines
    #    cv2.line(frame, (i * tile_width, padding), (i * tile_width, padding + game_height), (0,0,0), 1)
    #for i in range(0, 10): # drawing horizontal lines
    #    cv2.line(frame, (0, padding + int(tile_width / 2) + (i * tile_width)), (game_width, padding + int(tile_width / 2) + (i * tile_width)), (0,0,0), 1)
    
    #print("Detection complete")
    cv2.imshow("Screen", frame)
    status = "ok"
    if cv2.waitKey(1) == ord('q'):
        status = "quit"
    return status, predictions_for_map, False

actions = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,2,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2]
actions = [2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,0,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
actions = [3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,1,4,4,4,4,1,1,1,1,1,1,1,1,1,1,1]
actions = [1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,4,1,4,4,4,4,3,3,3,3,3,3,3,3]

if __name__ == "__main__":
    # Setup variables here
    game_width = 720
    game_height = 480
    game_window = np.zeros((game_height, game_width, 4), "uint8")
    model_path = "../inference_graphs/400p/resnet101_csv_13.h5"
    labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"}

    # Initialising model, window, and controller
    ctrl, window_x, window_y, model, mp = initialise(game_window, game_width, game_height, model_path)

    is_init_frame = True
    predictions_for_map = []
    temp_bool = None
    key_pressed = None
    
    map_grid = np.full((2, 2), 255, dtype=np.uint8)
    four_frame_count = 0

    action_index = -1
    while True:  
        if (four_frame_count == 0):
            time.sleep(0.5)
            action_index += 1
            if (action_index >= len(actions)):
                break

            if (is_init_frame == True):
                ctrl.dummy()
                key_pressed = None
                framerate_start = time.time()
                frame, temp = get_screen(game_window, window_x, window_y)
                status, predictions_for_map, temp_init = run_detection(frame, model, labels_to_names, mp)
                four_frame_count += 1
                framerate = time.time() - framerate_start
                #print(framerate)
                map_grid = mp.draw_init_map(key_pressed, predictions_for_map)
                print(map_grid)
                cv2.imshow("Map", map_grid)

            else:
                key_pressed = ctrl.random_movement(action=actions[action_index])
            print(key_pressed)
            four_frame_count += 1
            #ctrl.dummy()
        elif (four_frame_count < 4):
            framerate_start = time.time()
            frame, temp = get_screen(game_window, window_x, window_y)
            status, predictions_for_map, temp_bool = run_detection(frame, model, labels_to_names, mp)
            four_frame_count += 1
            framerate = time.time() - framerate_start
            #print(framerate)

            if (status == "quit"):
                break

        elif (four_frame_count == 4):
            framerate_start = time.time()
            frame, temp = get_screen(game_window, window_x, window_y)
            status, predictions_for_map, temp_bool = run_detection(frame, model, labels_to_names, mp)
            framerate = time.time() - framerate_start
            #print(framerate)

            map_grid = mp.draw_map(key_pressed, predictions_for_map)
            print(map_grid)
            cv2.imshow("Map", map_grid)
            four_frame_count = 0
        
        

        if (is_init_frame == False):
            pass
            #cv2.imshow("Map", map_grid)
        else:
            is_init_frame = temp_bool      

    cv2.destroyAllWindows()
    sys.exit()
