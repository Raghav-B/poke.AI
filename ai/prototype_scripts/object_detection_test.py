import keras
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color
import tensorflow as tf

import pyautogui as pag

import cv2
import numpy as np
from mss import mss
from PIL import Image
import time

def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)
keras.backend.tensorflow_backend.set_session(get_session())

model_path = "../../object_detection/keras-retinanet/inference_graphs/resnet50_csv_20.h5" # Model to be used for detection
labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit", 6: "wall", 7:"grass"} # Labels to draw
model = models.load_model(model_path, backbone_name='resnet50')

game_window_size = {"top": 0, "left": 0, "width": 720, "height": 480}
game_window_size["left"], game_window_size["top"], temp1, temp2 = pag.locateOnScreen("../find_game_window_windows.png")
game_window_size["top"] += 76

sct = mss()

while (True):
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

    # preprocess image for network
    image = preprocess_image(frame)
    image, scale = resize_image(image, min_side=400)

    boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0))
    boxes /= scale

    # visualize detections
    for box, score, label in zip(boxes[0], scores[0], labels[0]):
        if score < (80 / 100):
            break
        
        color = label_color(label)
        b = box.astype(int)
        draw_box(frame, b, color=color)
        caption = "{} {:.2f}".format(labels_to_names[label], score)
        draw_caption(frame, b, caption)

    cv2.imshow("Screen", frame)

    if cv2.waitKey(1) == ord('q'):
        break

cv2.destroyAllWindows()