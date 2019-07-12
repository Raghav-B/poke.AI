import cv2
import numpy as np

test_img = cv2.imread("hqdefault.jpg")
#test_img = cv2.imread("test.png")

def nothing(x):
    pass

cv2.namedWindow("Output")
cv2.createTrackbar("low_h", "Output", 68, 179, nothing)
cv2.createTrackbar("low_s", "Output", 118, 255, nothing)
cv2.createTrackbar("low_v", "Output", 199, 255, nothing)
cv2.createTrackbar("high_h", "Output", 76, 179, nothing)
cv2.createTrackbar("high_s", "Output", 156, 255, nothing)
cv2.createTrackbar("high_v", "Output", 255, 255, nothing)

# Max health for each pokemon is 128

while True:
    test_img = cv2.resize(test_img, (720, 480))

    lh = cv2.getTrackbarPos("low_h", "Output")
    ls = cv2.getTrackbarPos("low_s", "Output")
    lv = cv2.getTrackbarPos("low_v", "Output")
    hh = cv2.getTrackbarPos("high_h", "Output")
    hs = cv2.getTrackbarPos("high_s", "Output")
    hv = cv2.getTrackbarPos("high_v", "Output")

    lower_bound = (lh, ls, lv)
    upper_bound = (hh, hs, hv)

    output_img = cv2.cvtColor(test_img, cv2.COLOR_BGR2HSV)
    output_img = cv2.inRange(output_img, lower_bound, upper_bound)
    
    input_img = test_img.copy()

    contours, hierarchy = cv2.findContours(output_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cv2.drawContours(input_img, contours, -1, (0, 0, 255), 1)

    cv2.rectangle(input_img, (510, 370), (680, 420), (255, 0, 0))
    cv2.rectangle(input_img, (140, 190), (310, 240), (255, 0, 0))

    for cnt in contours:
        leftmost = tuple(cnt[cnt[:,:,0].argmin()][0])
        rightmost = tuple(cnt[cnt[:,:,0].argmax()][0])
        topmost = tuple(cnt[cnt[:,:,1].argmin()][0])
        bottommost = tuple(cnt[cnt[:,:,1].argmax()][0])

        print(rightmost[0] - leftmost[0])
    print("")

    cv2.imshow("Input", input_img)
    cv2.imshow("Output", output_img)
    cv2.waitKey(1)

"""
Actions:

Move 1:
Z, Z

Move 2:
Z, 

Things to do:
- Collect data from

"""