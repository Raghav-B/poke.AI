import keras
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color
import tensorflow as tf

import cv2
import numpy as np
from mss import mss
from PIL import Image
import time

# Custom imports
from sort_midpoints import midpoint_sorter

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
frame = None
gameplay_window_size = {}

init_screen_size = {'top': 0, 'left': 0, 'width': 1280, 'height': 720}

def nothing(x):
    pass

def draw_rect(event, x, y, flags, param):
    global now_drawing
    global start_pos
    global end_pos
    global frame
    global current_step
    global gameplay_window_size

    if (current_step == 0):
        if event == cv2.EVENT_LBUTTONDOWN:
            now_drawing = True
            start_pos = (x, y)
        elif event == cv2.EVENT_MOUSEMOVE:
            if (now_drawing == True):
                frame = cv2.rectangle(frame, start_pos, (x, y), (0, 255, 0), 1)
        elif event == cv2.EVENT_LBUTTONUP:
            now_drawing = False
            end_pos = (x, y)
            current_step = 1
            gameplay_window_size = {"top": start_pos[1], "left": start_pos[0],\
            "width": end_pos[0]-start_pos[0], "height": end_pos[1]-start_pos[1]}

    cv2.imshow("Screen", frame)

cv2.namedWindow("Screen")
cv2.setMouseCallback("Screen", draw_rect)
cv2.createTrackbar("FrameSkip", "Screen", 0, 15, nothing)
cv2.createTrackbar("ScoreThresh", "Screen", 70, 99, nothing)

prev_frame_midpoints = []
cur_frame_midpoints = []
is_init_frame = True

sct = mss()
ot = midpoint_sorter() # object tracking across consequent frames
while (True):
    if (current_step == 0):
        sct.get_pixels(init_screen_size)
        img = Image.frombytes('RGB', (sct.width, sct.height), sct.image)
        frame = np.array(img)
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

    elif (current_step == 1):
        frame_skip_amt = cv2.getTrackbarPos("FrameSkip", "Screen")
        score_thresh = cv2.getTrackbarPos("ScoreThresh", "Screen")

        for i in range(0, frame_skip_amt):
            sct.get_pixels(gameplay_window_size)
            img = Image.frombytes("RGB", (sct.width, sct.height), sct.image)
            frame = np.array(img)
            frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
        
        sct.get_pixels(gameplay_window_size)
        img = Image.frombytes("RGB", (sct.width, sct.height), sct.image)
        frame = np.array(img)
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

        # Making input a square
        rows = frame.shape[0]
        cols = frame.shape[1]
        #print(frame.shape)
        if rows < cols:
            padding = int((cols - rows) / 2)
            frame = cv2.copyMakeBorder(frame, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
        elif rows > cols:
            padding = int((rows - cols) / 2)
            frame = cv2.copyMakeBorder(frame, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))

        # preprocess image for network
        image = preprocess_image(frame)
        image, scale = resize_image(image, min_side = 720)

        boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0))
        boxes /= scale

        # visualize detections
        for box, score, label in zip(boxes[0], scores[0], labels[0]):
            if score < (score_thresh / 100):
                break
            
            midpoint_x = int((box[2] + box[0]) / 2)
            midpoint_y = int((box[3] + box[1]) / 2)
            
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

        # Sorting cur_frame midpoints
        if (is_init_frame == False):
            print("prev_frame")
            print(prev_frame_midpoints)
            print("cur_frame")
            print(cur_frame_midpoints)
            print("")
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
        is_init_frame = False

    if (now_drawing == False):
        cv2.imshow("Screen", frame)

    if cv2.waitKey(1) == ord('q'):
        break

cv2.destroyAllWindows()