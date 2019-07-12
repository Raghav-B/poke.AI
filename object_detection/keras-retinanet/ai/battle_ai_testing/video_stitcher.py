import cv2
import glob
import os

cur_frames = glob.glob("gameplay_frames/*.jpg")

def sort_key_func(path):
    new_path = os.path.basename(path)[:-4]
    return int(new_path)

print("Sorting frames")
cur_frames.sort(key=sort_key_func)

print(cur_frames)

output_video = cv2.VideoWriter("battle_video45.mp4", 0x00000021, 45.0, (720, 720))
    
print("Stitching frames...")

for x in cur_frames:
    y = cv2.imread(x)
    output_video.write(y)
    print("Frame " + x + " written.")

output_video.release()