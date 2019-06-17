import keras
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color

import cv2
import numpy as np

import matplotlib.pyplot as plt
import os
import glob

import tensorflow as tf

def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)
keras.backend.tensorflow_backend.set_session(get_session())

model_path = os.path.join('inference_graphs', 'resnet101_csv_05.h5')
model = models.load_model(model_path, backbone_name='resnet101')

labels_to_names = {0: "normal_house", 1: "pokemon_center", 2: "pokemart", 3: "gym", 4: "misc", 5: "cave"}

video = cv2.VideoCapture("../videos/gameplay_reduced.mp4")
total_frames = int(video.get(cv2.CAP_PROP_FRAME_COUNT))
frame_skip_amt = 2

cur_frame = 0
fourcc = cv2.VideoWriter_fourcc(*"XVID")
output_video = cv2.VideoWriter("../videos/output.avi", fourcc, 30.0, (720, 480))

while(True):
    # Performing frame-skip for increased performance
    #for i in range(0, frame_skip_amt):
    #    ret, frame = video.read()
    ret, frame = video.read()
    
    draw = frame.copy()
    draw = cv2.cvtColor(draw, cv2.COLOR_BGR2RGB)

    # preprocess image for network
    image = preprocess_image(frame)
    image, scale = resize_image(image)

    # process image
    boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0))

    # correct for image scale
    boxes /= scale

    # visualize detections
    for box, score, label in zip(boxes[0], scores[0], labels[0]):
        if score < 0.7:
            break
        
        color = label_color(label)
        
        b = box.astype(int)
        draw_box(draw, b, color=color)
        
        caption = "{} {:.3f}".format(labels_to_names[label], score)
        draw_caption(draw, b, caption)
    
    output_img = cv2.cvtColor(draw, cv2.COLOR_RGB2BGR)
    #cv2.imshow("Detection", output_img)

    output_video.write(output_img)
    cur_frame += 1
    print("Finished " + str(cur_frame) + " out of " + str(total_frames) + " total frames...")

    if cv2.waitKey(1) == ord('q'): # Save current frame
        break

output_video.release()
video.release()
cv2.destroyAllWindows()