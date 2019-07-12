import keras
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color
import tensorflow as tf

import cv2
import numpy as np
from mss import mss
import pyautogui as pag

# Some keras/tensorflow related stuff, even I'm not entirely sure what it does exactly
def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)

model_path = "../../inference_graphs/400p/resnet101_csv_13.h5" # Model to be used for detection
labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"} # Labels to draw

keras.backend.tensorflow_backend.set_session(get_session())
model = models.load_model(model_path, backbone_name='resnet101')

video = cv2.VideoCapture("gameplay_frames_video/battle_video30.mp4")
ret, frame = video.read()

while ret:
    ret, frame = video.read() 

    # Process image and run inference
    image = preprocess_image(frame) # Retinanet specific preprocessing
    image, scale = resize_image(image, min_side = 400) # This model was trianed with 400p images
    boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0)) # Run inference
    boxes /= scale # Ensures bounding boxes are of the correct scale

    has_detections = False

    for box, score, label in zip(boxes[0], scores[0], labels[0]):
        # We can break here because the bounding boxes are in descending order in terms of confidence
        if score < (0.7):
            break
        
        has_detections = True

        # Drawing labels and bounding boxes on input frame
        color = label_color(label)
        b = box.astype(int)
        draw_box(frame, b, color=color)
        caption = "{} {:.2f}".format(labels_to_names[label], score)
        draw_caption(frame, b, caption)
    
    if (has_detections == False):
        print("No objects detected!")
    else:
        print("Detected objects!")

    cv2.imshow("Screen", frame)
    key = cv2.waitKey()
    if key == ord('q'):
        break
    else:
        continue

# Clean running processes and close program cleanly
cv2.destroyAllWindows()
sys.exit()
    
    

    
