import cv2
import glob

imgs_to_pad = glob.glob("large_training/*.jpg")
cur_index = 0

for img_path in imgs_to_pad:
    input_img = cv2.imread(img_path)

    try:
        rows = input_img.shape[0]
        cols = input_img.shape[1]

        if rows < cols:
            padding = int((cols - rows) / 2)
            input_img = cv2.copyMakeBorder(input_img, padding, padding, 0, 0, cv2.BORDER_CONSTANT, (0, 0, 0))
        elif rows > cols:
            padding = int((rows - cols) / 2)
            input_img = cv2.copyMakeBorder(input_img, 0, 0, padding, padding, cv2.BORDER_CONSTANT, (0, 0, 0))
        else:
            cur_index += 1
            print("Finished padding " + str(cur_index) + " out of " + str(len(imgs_to_pad)) + " images...")
            continue

        cv2.imwrite(img_path, input_img)

        cur_index += 1
        print("Finished padding " + str(cur_index) + " out of " + str(len(imgs_to_pad)) + " images...")
    except AttributeError:
        continue

print("Padding finished")