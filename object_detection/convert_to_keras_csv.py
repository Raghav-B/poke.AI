import numpy as np
import pandas as pd
import os

csv_file = pd.read_csv("split_outputs/new_train.csv")
cur_dir = os.getcwd()
print(cur_dir)

new_paths = csv_file["filename"]
new_paths = new_paths.copy()

loop_index = 0
for i in new_paths:
	temp = os.path.join(cur_dir + str(i))
	new_paths[loop_index] = temp
	loop_index += 1

print(new_paths)

new_dataframe = pd.concat([new_paths, csv_file["xmin"], csv_file["ymin"], csv_file["xmax"],\
csv_file["ymax"], csv_file["class"]], axis = 1)

new_dataframe.to_csv("keras_train.csv", index = False)