import cv2
import numpy as np
import glob
import os
import pandas as pd

output_dir = "split_training_maps/"
image_list = glob.glob("training_maps/*.png")
csv_file = pd.read_csv("training_csvs/train.csv")
split_data = []

for i in range(len(image_list)):
	cur_image = cv2.imread(image_list[i])
	only_image_name = os.path.split(image_list[i])

	#REMEMBER: COLS IS X AXIS AND ROWS IS Y AXIS
	rows = cur_image.shape[0]
	cols = cur_image.shape[1]
	top_left = [[0, int((rows / 2))], [0, int((cols / 2))]] 
	top_right = [[0, int((rows / 2))], [int((cols / 2) + 1), cols]]
	bot_left = [[int((rows / 2) + 1), rows], [0, int((cols / 2))]]
	bot_right = [[int((rows / 2) + 1), rows], [int((cols / 2) + 1), cols]]
	
	segments = [top_left, top_right, bot_left, bot_right]
	segment_frames = []

	crop_names = ["top_left_", "top_right_", "bot_left_", "bot_right_"]
	
	# Data frame for current image to be cropped
	cur_data = csv_file[csv_file["filename"] == only_image_name[1]]
	
	# Iterating through 4 cropped regions and checking if labels fall inside any one of these
	for seg in range(4):
		
		# Iterating through current image's data frame to relabel new cropped regions
		for j in range(cur_data.shape[0]):
			
			# Getting original coordinates of original labels
			j_coords = [cur_data.iloc[j][4], cur_data.iloc[j][5], cur_data.iloc[j][6], cur_data.iloc[j][7]]
			
			# Checking if any of the 4 coordinates fall inside current cropped region
			if ((j_coords[0] >= segments[seg][1][0] and j_coords[0] <= segments[seg][1][1]\
			and j_coords[1] >= segments[seg][0][0] and j_coords[1] <= segments[seg][0][1])\
			or\
			(j_coords[2] >= segments[seg][1][0] and j_coords[2] <= segments[seg][1][1]\
			and j_coords[1] >= segments[seg][0][0] and j_coords[1] <= segments[seg][0][1])\
			or\
			(j_coords[0] >= segments[seg][1][0] and j_coords[0] <= segments[seg][1][1]\
			and j_coords[3] >= segments[seg][0][0] and j_coords[3] <= segments[seg][0][1])\
			or\
			(j_coords[2] >= segments[seg][1][0] and j_coords[2] <= segments[seg][1][1]\
			and j_coords[3] >= segments[seg][0][0] and j_coords[3] <= segments[seg][0][1])):
				
				thresh_area = 0.25 * (j_coords[2] - j_coords[0]) * (j_coords[3] - j_coords[1])
				
				# Splices boxes that intersect with cropped regions
				if (j_coords[0] <= segments[seg][1][0]):
					j_coords[0] = segments[seg][1][0]
				if (j_coords[2] >= segments[seg][1][1]):
					j_coords[2] = segments[seg][1][1]
				if (j_coords[1] <= segments[seg][0][0]):
					j_coords[1] = segments[seg][0][0]
				if (j_coords[3] >= segments[seg][0][1]):
					j_coords[3] = segments[seg][0][1]
				
				# Filtering out label cutouts that are too small (side is one-tenth of image side)
				#if ((j_coords[2] - j_coords[0]) <= ((cols / 2) / 10)\
				#or (j_coords[3] - j_coords[1]) <= ((rows / 2) / 10)):
				#	continue
				
				# Filtering using 50% total area metric
				if ((j_coords[2] - j_coords[0]) * (j_coords[3] - j_coords[1]) < thresh_area):
					continue
				
				# Normalizing label coordinates post-crop
				if (seg == 1): #top_right
					j_coords[0] -= int(cols / 2)
					j_coords[2] -= int(cols / 2) + 1
				elif (seg == 2): #bot_left
					j_coords[1] -= int(rows / 2)
					j_coords[3] -= int(rows / 2) + 1
				elif (seg == 3): #bot_right
					j_coords[0] -= int(cols / 2)
					j_coords[2] -= int(cols / 2) + 1
					j_coords[1] -= int(rows / 2)
					j_coords[3] -= int(rows / 2) + 1

				# Adding new row of data to list
				segment_frames.append([(crop_names[seg] + only_image_name[1]),\
				(int(cols / 2)), (int(rows / 2)), cur_data.iloc[j][3],\
				j_coords[0], j_coords[1], j_coords[2], j_coords[3]])
	
	split_data.extend(segment_frames)
	
	# Cropping into 4 regions
	crop1 = cur_image[top_left[0][0]:top_left[0][1], top_left[1][0]:top_left[1][1]]
	crop2 = cur_image[top_right[0][0]:top_right[0][1], top_right[1][0]:top_right[1][1]]
	crop3 = cur_image[bot_left[0][0]:bot_left[0][1], bot_left[1][0]:bot_left[1][1]]
	crop4 = cur_image[bot_right[0][0]:bot_right[0][1], bot_right[1][0]:bot_right[1][1]]
	cv2.imwrite(output_dir + "top_left_" + only_image_name[1], crop1)
	cv2.imwrite(output_dir + "top_right_" + only_image_name[1], crop2)
	cv2.imwrite(output_dir + "bot_left_" + only_image_name[1], crop3)
	cv2.imwrite(output_dir + "bot_right_" + only_image_name[1], crop4)
	
	print("Finished " + str(i + 1) + " out of " + str(len(image_list)) + " images.")

final_data_frame = pd.DataFrame(split_data, columns = ["filename", "width", "height", "class", "xmin", "ymin", "xmax", "ymax"])
final_data_frame.to_csv("training_csvs/split_train.csv", index = False)