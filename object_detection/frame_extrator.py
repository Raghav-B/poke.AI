import numpy as np
import cv2

input_vid = cv2.VideoCapture("gameplay_reduced.mp4")
total_frames = int(input_vid.get(cv2.CAP_PROP_FRAME_COUNT))

frame_index = 0
skip_frames = 100

ret = True

while (ret):
    for i in range(1, skip_frames):
        ret, frame = input_vid.read()

    ret, frame = input_vid.read()

    output_path = "../training_gameplay/" + str(frame_index) + ".jpg"
    #print(output_path)
    
    cv2.imwrite(output_path, frame)

    frame_index += 1
    print("Extracted " + str(frame_index) + " out of " + str(total_frames) + " frames...")
    
    #if frame_index == 10001:
    #    break

print("Extraction completed.")
input_vid.release()
