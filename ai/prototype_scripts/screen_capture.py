import numpy as np
import cv2
from mss import mss
from PIL import Image

rect = []
current_step = 0
now_drawing = False
start_pos = ()
end_pos = ()
frame = None
gameplay_window_size = {}

init_screen_size = {'top': 0, 'left': 0, 'width': 1280, 'height': 720}

def draw_rect(event, x, y, flags, param):
    global now_drawing
    global start_pos
    global end_pos
    global frame
    global current_step
    global gameplay_window_size

    if (current_step == 0):
        if event == cv2.EVENT_LBUTTONDOWN:
            now_drawing = True
            start_pos = (x, y)
        elif event == cv2.EVENT_MOUSEMOVE:
            if (now_drawing == True):
                frame = cv2.rectangle(frame, start_pos, (x, y), (0, 255, 0), 1)
        elif event == cv2.EVENT_LBUTTONUP:
            now_drawing = False
            end_pos = (x, y)
            current_step = 1
            gameplay_window_size = {"top": start_pos[1], "left": start_pos[0],\
            "width": end_pos[0]-start_pos[0], "height": end_pos[1]-start_pos[1]}

    cv2.imshow("Screen", frame)

cv2.namedWindow("Screen")
cv2.setMouseCallback("Screen", draw_rect)

sct = mss()
while True:
    if (current_step == 0):
        sct.get_pixels(init_screen_size)
        img = Image.frombytes('RGB', (sct.width, sct.height), sct.image)
        frame = np.array(img)
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

    elif (current_step == 1):
        sct.get_pixels(gameplay_window_size)
        img = Image.frombytes("RGB", (sct.width, sct.height), sct.image)
        frame = np.array(img)
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

    if (now_drawing == False):
        cv2.imshow("Screen", frame)
       
    if cv2.waitKey(1) == ord('q'):
        break

cv2.destroyAllWindows()