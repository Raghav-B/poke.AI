import keras
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color
import tensorflow as tf

import cv2
import numpy as np
from fastgrab._linux_x11 import screenshot
import pyautogui as pag
import time

# Custom imports
from sort_midpoints import midpoint_sorter
from mapper import live_map

def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)
keras.backend.tensorflow_backend.set_session(get_session())

model_path = "../inference_graphs/resnet101_csv_09.h5"
model = models.load_model(model_path, backbone_name='resnet101')

labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"}

rect = []
current_step = 0
now_drawing = False
start_pos = ()
end_pos = ()

# Setup variables here
window_x = 0 # Initialized here
window_y = 0 # Initialized here
game_width = 720
game_height = 480
game_window = np.zeros((game_height, game_width, 4), "uint8")
tile_width = int(game_width / 15) # This 15 is always constant
padding = 0 # Initialized here

def nothing(x):
    pass

def draw_rect(event, x, y, flags, param):
    global current_step
    if (current_step == 0):
        if event == cv2.EVENT_LBUTTONDBLCLK:
            current_step = 1
        
cv2.namedWindow("Screen")
#cv2.setMouseCallback("Screen", draw_rect)
cv2.createTrackbar("FrameSkip", "Screen", 0, 15, nothing)
cv2.createTrackbar("ScoreThresh", "Screen", 70, 99, nothing)

is_init_frame = True
ot = midpoint_sorter() # object tracking across consequent frames
prev_frame_midpoints = []
cur_frame_midpoints = []

mp = None
predictions_for_map = []

while (True):
    if (current_step == 0):
        window_x, window_y, temp1, temp2 = pag.locateOnScreen("find_game_window.png")
        # Adding a 20 pixel offset to the y coordinate since our gameplay is slightly lower.
        window_y += 20
        current_step = 1

    elif (current_step == 1):
        frame_skip_amt = cv2.getTrackbarPos("FrameSkip", "Screen")
        score_thresh = cv2.getTrackbarPos("ScoreThresh", "Screen")

        # Getting game screen as input
        for i in range(0, frame_skip_amt):
            screenshot(window_x, window_y, game_window)
        screenshot(window_x, window_y, game_window)
        frame = game_window[:, :, :3]

        # Making input a square
        if game_height < game_width:
            padding = int((game_width - game_height) / 2)
            frame = cv2.copyMakeBorder(frame, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
            #print(padding)
        elif game_height > game_width:
            padding = int((game_width - game_width) / 2)
            frame = cv2.copyMakeBorder(frame, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))

        # Initialising live map object
        if (is_init_frame == True):
            mp = live_map(game_width, game_height, padding)

        # preprocess image for network
        image = preprocess_image(frame)
        image, scale = resize_image(image, min_side = 720)

        boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0))
        boxes /= scale

        # visualize detections
        predictions_for_map = []
        for box, score, label in zip(boxes[0], scores[0], labels[0]):
            if score < (score_thresh / 100):
                break
            
            midpoint_x = int((box[2] + box[0]) / 2)
            midpoint_y = int((box[3] + box[1]) / 2)
            
            predictions_for_map.append((label, box))

            if (is_init_frame == True):
                # Indexing previous frame
                prev_frame_midpoints.append([(midpoint_x, midpoint_y), ot.get_init_index()])
            else:
                cur_frame_midpoints.append([(midpoint_x, midpoint_y), -1])

            color = label_color(label)
            b = box.astype(int)
            draw_box(frame, b, color=color)
            caption = "{} {:.2f}".format(labels_to_names[label], score)
            draw_caption(frame, b, caption)
        #print(len(cur_frame_midpoints))

        # Sorting cur_frame midpoints
        if (is_init_frame == False):
            cur_frame_midpoints = ot.sort_cur_midpoints(prev_frame_midpoints, cur_frame_midpoints)

        font = cv2.FONT_HERSHEY_SIMPLEX
        for point in cur_frame_midpoints:
            #frame = cv2.circle(frame, (point[1][0], point[1][1]), 3, (0, 255, 0), -1)
            cv2.putText(frame, str(point[1]), point[0], font, 0.5, (255, 255, 0), 2, cv2.LINE_AA)
        
        if (is_init_frame == False):
            # current frame becomes previous frame
            prev_frame_midpoints = cur_frame_midpoints
            # current frame can be reset because we will detect points again next frame
            cur_frame_midpoints = []

            #print(predictions_for_map)
            mp.draw_map(predictions_for_map)
        is_init_frame = False

        for i in range(0, 16): # drawing vertical lines
            cv2.line(frame, (i * tile_width, padding), (i * tile_width, padding + game_height), (0,0,0), 1)
        for i in range(0, 10): # drawing horizontal lines
            cv2.line(frame, (0, padding + int(tile_width / 2) + (i * tile_width)), (game_width, padding + int(tile_width / 2) + (i * tile_width)), (0,0,0), 1)

        cv2.imshow("Screen", frame)

        if cv2.waitKey(1) == ord('q'):
            break

cv2.destroyAllWindows()