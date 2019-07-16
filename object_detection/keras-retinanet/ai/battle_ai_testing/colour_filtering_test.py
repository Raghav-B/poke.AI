import cv2
import numpy as np
from mss import mss
import pyautogui as pag

def nothing(x):
    pass

cv2.namedWindow("Output")
cv2.createTrackbar("low_h", "Output", 45, 179, nothing)
cv2.createTrackbar("low_s", "Output", 26, 255, nothing)
cv2.createTrackbar("low_v", "Output", 94, 255, nothing)
cv2.createTrackbar("high_h", "Output", 68, 179, nothing)
cv2.createTrackbar("high_s", "Output", 62, 255, nothing)
cv2.createTrackbar("high_v", "Output", 108, 255, nothing)
cv2.createTrackbar("area_thresh", "Output", 0, 20000, nothing)

video = cv2.VideoCapture("gameplay_frames_video/battle_video30.mp4")
ret, frame = video.read()

while ret:
    ret, frame = video.read() 
    cv_frame = frame.copy()

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
        area_t = cv2.getTrackbarPos("area_thresh", "Output")

        lower_bound = (lh, ls, lv)
        upper_bound = (hh, hs, hv)

        output_img = cv2.cvtColor(cv_frame, cv2.COLOR_BGR2HSV)
        output_img = cv2.inRange(output_img, lower_bound, upper_bound)

        input_img = cv_frame.copy()

        contours, hierarchy = cv2.findContours(output_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        #cv2.drawContours(input_img, contours, -1, (0, 0, 255), 1)

        my_hp = 0
        opp_hp = 0

        my_hp_detected = False
        opp_hp_detected = False
        for cnt in contours:
            if (cv2.contourArea(cnt) > area_t):
                M = cv2.moments(cnt)
                centroid_x = int(M['m10']/M['m00'])
                centroid_y = int(M['m01']/M['m00'])

                # My HP
                if (centroid_x >= 510 and centroid_x <= 680 and centroid_y >= 370 and centroid_y <= 420):
                    my_hp_detected = True
                    leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                    rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                    cv2.drawContours(input_img, [cnt], 0, (0, 0, 255), 1)
                    cv2.circle(input_img, (centroid_x, centroid_y), 20, (0, 0, 255), -1)

                # Opponenet's HP
                elif (centroid_x >= 140 and centroid_x <= 310 and centroid_y >= 200 and centroid_y <= 250):
                    opp_hp_detected = True
                    leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
                    rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
                    cv2.drawContours(input_img, [cnt], 0, (0, 0, 255), 1)
                    cv2.circle(input_img, (centroid_x, centroid_y), 20, (0, 0, 255), -1)

        cv2.rectangle(input_img, (510, 370), (680, 420), (255, 0, 0))
        cv2.rectangle(input_img, (140, 200), (310, 250), (255, 0, 0))

        print(my_hp_detected, opp_hp_detected)
        print("")

        cv2.imshow("Input", input_img)
        cv2.imshow("Output", output_img)

        key = cv2.waitKey(1)
        if (key == ord('n')):
            break
        else:
            continue

# Clean running processes and close program cleanly
cv2.destroyAllWindows()
sys.exit()
    
    

    
