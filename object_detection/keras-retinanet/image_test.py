# import keras
import keras
import glob

# import keras_retinanet
from keras_retinanet import models
from keras_retinanet.utils.image import read_image_bgr, preprocess_image, resize_image
from keras_retinanet.utils.visualization import draw_box, draw_caption
from keras_retinanet.utils.colors import label_color

# import miscellaneous modules
import matplotlib.pyplot as plt
import cv2
import os
import numpy as np
import time

# set tf backend to allow memory to grow, instead of claiming everything
import tensorflow as tf

def get_session():
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    return tf.Session(config=config)

# use this environment flag to change which GPU to use
#os.environ["CUDA_VISIBLE_DEVICES"] = "1"

# set the modified tf session as backend in keras
keras.backend.tensorflow_backend.set_session(get_session())

#model_path = "inference_graphs/no_random_transform/resnet101_csv_50.h5"
model_path = "inference_graphs/resnet101_csv_09.h5"
model = models.load_model(model_path, backbone_name='resnet101')

labels_to_names = {0: "pokecen", 1: "pokemart", 2: "npc", 3: "house", 4: "gym", 5: "exit"}

input_images = glob.glob("training_inputs/*.jpg")
input_images_png = glob.glob("training_inputs/*.png")

#input_images = glob.glob("test_inputs/*.jpg")
#input_images_png = glob.glob("test_inputs/*.png")
input_images.extend(input_images_png)

index = 0
len_inputs = len(input_images)
for i in input_images:
    image = cv2.imread(i)
    image_name = os.path.split(i)

    draw = image.copy()
    draw = cv2.cvtColor(draw, cv2.COLOR_BGR2RGB)

    # preprocess image for network
    image = preprocess_image(image)
    image, scale = resize_image(image)

    boxes, scores, labels = model.predict_on_batch(np.expand_dims(image, axis=0))
    # correct for image scale
    boxes /= scale

    for box, score, label in zip(boxes[0], scores[0], labels[0]):
        if score < 0.7:
            break
            
        color = label_color(label)
        
        b = box.astype(int)
        draw_box(draw, b, color=color)
        
        caption = "{} {:.3f}".format(labels_to_names[label], score)
        draw_caption(draw, b, caption)

    draw = cv2.cvtColor(draw, cv2.COLOR_RGB2BGR)
    cv2.imwrite("test_outputs/output_" + image_name[1], draw)

    index += 1
    print("Completed " + str(index) + " out of " + str(len_inputs) + " images.") 