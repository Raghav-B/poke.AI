import cv2
import glob
import os

imgs = glob.glob("training_maps/*.DIB")

index = 0
for i in imgs:
    output = cv2.imread(i)
    output_name = os.path.split(i)
    cv2.imwrite("training_maps/" + output_name[1] + ".png", output)

    index += 1
    print("Completed " + str(index) + " out of " + str(len(imgs)) + " images.")
