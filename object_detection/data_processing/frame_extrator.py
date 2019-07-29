import numpy as np
import cv2
import random

input_vid = cv2.VideoCapture("videos/complete_gameplay.mp4")
total_frames = int(input_vid.get(cv2.CAP_PROP_FRAME_COUNT))

frame_index = 394
actual_frame_index = 0
skip_frames = 920

ret = True

while (ret):
    skip_frames = random.randint(100, 2500)
    print("Skipping " + str(skip_frames) + " frames...")
    

    for i in range(1, skip_frames):
        ret, frame = input_vid.read()
        actual_frame_index += 1
        #print("Skipped " + str(i) + " out of " + str(skip_frames) + " frames...")

    ret, frame = input_vid.read()
    print("Current frame: " + str(actual_frame_index) + " out of " + str(total_frames))

    output_path = "large_training/" + str(frame_index) + ".jpg"
    #print(output_path)
    
    cv2.imwrite(output_path, frame)

    frame_index += 1
    actual_frame_index += 1
    print("Extracted " + str(frame_index) + " out of " + str(1000) + " frames...")
    
    if frame_index == 2000:
        break

print("Extraction completed.")
input_vid.release()
