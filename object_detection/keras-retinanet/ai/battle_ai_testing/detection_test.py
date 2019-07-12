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

def nothing(x):
    pass

cv2.namedWindow("Output")
cv2.createTrackbar("low_h", "Output", 68, 179, nothing)
cv2.createTrackbar("low_s", "Output", 118, 255, nothing)
cv2.createTrackbar("low_v", "Output", 199, 255, nothing)
cv2.createTrackbar("high_h", "Output", 76, 179, nothing)
cv2.createTrackbar("high_s", "Output", 156, 255, nothing)
cv2.createTrackbar("high_v", "Output", 255, 255, nothing)

video = cv2.VideoCapture("gameplay_frames_video/battle_video30.mp4")
ret, frame = video.read()

while ret:
    ret, frame = video.read() 
    cv_frame = frame.copy()

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

    left_most = None
    right_most = None

    while True:
        lh = cv2.getTrackbarPos("low_h", "Output")
        ls = cv2.getTrackbarPos("low_s", "Output")
        lv = cv2.getTrackbarPos("low_v", "Output")
        hh = cv2.getTrackbarPos("high_h", "Output")
        hs = cv2.getTrackbarPos("high_s", "Output")
        hv = cv2.getTrackbarPos("high_v", "Output")

        #lower_bound = (lh, ls, lv)
        #upper_bound = (hh, hs, hv)

        #black_lower_bound = (36, 98, 198)
        #black_upper_bound = (75, 156, 255)

        black_lower_bound = (87, 0, 0)
        black_upper_bound = (164, 74, 91)

        black_detection_img = cv2.cvtColor(cv_frame, cv2.COLOR_BGR2HSV)
        black_detection_img = cv2.inRange(black_detection_img, black_lower_bound, black_upper_bound)



        #output_img = cv2.cvtColor(cv_frame, cv2.COLOR_BGR2HSV)
        #output_img = cv2.inRange(output_img, lower_bound, upper_bound)

        input_img = cv_frame.copy()

        contours, hierarchy = cv2.findContours(black_detection_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        #cv2.drawContours(input_img, contours, -1, (0, 0, 255), 1)

        my_hp = 0
        opp_hp = 0

        my_hp_detected = False
        opp_hp_detected = False
        for cnt in contours:
            if (cv2.contourArea(cnt) > 0):
                M = cv2.moments(cnt)
                centroid_x = int(M['m10']/M['m00'])
                centroid_y = int(M['m01']/M['m00'])

                # My HP
                if (centroid_x >= 510 and centroid_x <= 680 and centroid_y >= 370 and centroid_y <= 420):
                    my_hp_detected = True
                    leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                    rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                    print((rightmost[0] - leftmost[0]))
                    my_hp = 141 - (rightmost[0] - leftmost[0])
                    cv2.drawContours(input_img, [cnt], 0, (0, 0, 255), 1)

                # Opponenet's HP
                elif (centroid_x >= 140 and centroid_x <= 310 and centroid_y >= 200 and centroid_y <= 250):
                    opp_hp_detected = True
                    leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                    rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                    opp_hp = 141 - (rightmost[0] - leftmost[0])
                    cv2.drawContours(input_img, [cnt], 0, (0, 0, 255), 1)

        if (my_hp_detected == False):
            my_hp = 141
        if (opp_hp_detected == False):
            opp_hp = 141
        if (my_hp < 0):
            my_hp = 0
        if (opp_hp < 0):
            opp_hp = 0

        cv2.rectangle(input_img, (510, 370), (680, 420), (255, 0, 0))
        cv2.rectangle(input_img, (140, 200), (310, 250), (255, 0, 0))

        print("My HP: " + str(my_hp))
        print("Opp HP: " + str(opp_hp))
        print("")

        cv2.imshow("Input", input_img)
        cv2.imshow("Output", black_detection_img)

        key = cv2.waitKey(1)
        if (key == ord('n')):
            break
        else:
            continue


    #if key == ord('q'):
    #    break
    #else:
    #    continue

# Clean running processes and close program cleanly
cv2.destroyAllWindows()
sys.exit()
    
    

    
