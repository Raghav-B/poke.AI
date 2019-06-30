import matplotlib.pyplot as plt
import numpy as np
import math

class live_map:
    # Variables to initialize
    window_width = 0
    window_height = 0 
    padding = 0 # Black bars used to make input square
    tile_size = 0 # real-world size of square tiles
    definite_frames_thresh = 4 # If object is detected continuously for 4 frames, then it is definitely a valid detection

    # Internals used by mapper
    prev_map_grid = None # The detected map represented as a 2D array
    cur_map_grid = None
    grid_x = 16 # Tiles in x axis + 1
    grid_y = 12 # Tiles in y axis + 1

    object_list = []

    map_cutout_width = 16
    map_cutout_height = 12
    map_cutout_x = 0
    map_cutout_y = 0

    def __init__(self, w, h, pad, dft=4):
        self.window_width = w
        self.window_height = h
        self.padding = pad
        self.tile_size = int(w / (self.grid_x - 1))
        self.definite_frames_thresh = dft
        self.prev_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), 255, dtype=np.uint8)

    # bounding_box_list is of shape (label, box_dimensions)
    def convert_points_to_grid(self, bounding_box_list):
        # Converts real-world coordinates to grid values used in-game

        tiles = []
        for label, box in bounding_box_list:
            coords = [0, 0, 0, 0]

            # x1
            q = box[0] / self.tile_size
            if abs(box[0] - (self.tile_size * math.floor(q))) < \
            abs(box[0] - (self.tile_size * math.ceil(q))):
                coords[0] = math.floor(q)
            else:
                coords[0] = math.ceil(q)

            # y1
            q = (box[1] - self.padding) / self.tile_size
            if abs((box[1] - self.padding + (self.tile_size/2)) - (self.tile_size * math.floor(q))) < \
            abs((box[1] - self.padding) - (self.tile_size * math.ceil(q))):
                coords[1] = math.floor(q)
            else:
                coords[1] = math.ceil(q)

            # x2
            q = box[2] / self.tile_size
            if abs(box[2] - (self.tile_size * math.floor(q))) < \
            abs(box[2] - (self.tile_size * math.ceil(q))):
                coords[2] = math.floor(q) - 1
            else:
                coords[2] = math.ceil(q) - 1

            # y2
            q = (box[3] - self.padding) / self.tile_size
            if abs((box[3] - self.padding + (self.tile_size/2)) - (self.tile_size * math.floor(q))) < \
            abs((box[3] - self.padding) - (self.tile_size * math.ceil(q))):
                coords[3] = math.floor(q) - 1
            else:
                coords[3] = math.ceil(q) - 1

            coords[0] -= self.map_cutout_x
            coords[1] -= self.map_cutout_y
            coords[2] -= self.map_cutout_x
            coords[3] -= self.map_cutout_y

            tiles.append((label, coords))
        return tiles

    def fill_area(self, area_bound, symbol):
        if (symbol == 0):
            self.cur_map_grid[area_bound[3]][area_bound[2]] = 0
        elif (symbol == 128):
            if (self.cur_map_grid[area_bound[1]][area_bound[0]] != 0):
                self.cur_map_grid[area_bound[1]][area_bound[0]] = 128
            if (self.cur_map_grid[area_bound[1]][area_bound[2]] != 0):
                self.cur_map_grid[area_bound[1]][area_bound[2]] = 128
        else:
            x = area_bound[0] 
            while (x <= area_bound[2]):
                y = area_bound[1]
                while (y <= area_bound[3]):
                    if (self.cur_map_grid[y][x] != 0):
                        self.cur_map_grid[y][x] = symbol
                    y += 1
                x += 1

    def add_to_object_list(self, key_pressed, bounding_box_list):
        tiles = self.convert_points_to_grid(bounding_box_list)
        
        self.cur_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), 255, dtype=np.uint8)
        if (key_pressed == "up"):
            if (self.map_cutout_y - 1 < 0):
                self.grid_y += 1
                append_arr = np.full((1, self.grid_x - 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(append_arr, self.cur_map_grid, axis=0)
                self.map_cutout_y = 0
            else:
                self.map_cutout_y -= 1
        
        elif (key_pressed == "right"):
            if (self.map_cutout_x + self.map_cutout_width > self.grid_x):
                self.grid_x += 1
                append_arr = np.full((self.grid_y - 1, 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(self.cur_map_grid, append_arr, axis=1)
                self.map_cutout_x += 1
            else:
                self.map_cutout_x += 1
            
        elif (key_pressed == "down"):
            if (self.map_cutout_y + self.map_cutout_height > self.grid_y):
                self.grid_y += 1
                append_arr = np.full((1, self.grid_x - 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(self.cur_map_grid, append_arr, axis=0)
                self.map_cutout_y += 1
            else:
                self.map_cutout_y += 1
            
        elif (key_pressed == "left"):
            if (self.map_cutout_x - 1 < 0):
                self.grid_x += 1
                append_arr = np.full((self.grid_y - 1, 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(append_arr, self.cur_map_grid, axis=1)
            else:
                self.map_cutout_x = 0

        else: # If key is "x"
            pass

        for label, box in self.object_list:
            if (key_pressed == "up"):
                box[1] += 1
                box[3] += 1
            elif (key_pressed == "right"):
                pass
            elif (key_pressed == "down"):
                pass
            elif (key_pressed == "left"):
                box[0] += 1
                box[2] += 1
            else:
                pass

        print(self.object_list)
        print(tiles)

        temp_object_list = []
        for new_label, new_box in tiles:
            
            is_found = False

            for label, box in self.object_list:
                if (new_box[0] == box[0] and new_box[1] == box[1]):
                    is_found == True
                    break
            
            if (is_found == False):
                temp_object_list.append((new_label, new_box))
        
        self.object_list.extend(temp_object_list)
        #return tiles

    # Returns true if a change has been detected, otherwise returns false.
    def draw_map(self, key_pressed, bounding_box_list):
        self.add_to_object_list(key_pressed, bounding_box_list)

        symbol = None
        has_map_changed = None

        #print(len(self.object_list))
        for label, box in self.object_list:
            if (label == 0): # pokecen
                symbol = 16
            elif (label == 1): # pokemart
                symbol = 32
            elif (label == 2): # npc
                symbol = 0
            elif (label == 3): # house
                symbol = 64
            elif (label == 4): # gym
                symbol = 48
            elif (label == 5): # exit
                symbol = 128    
            self.fill_area(box, symbol)

        has_map_changed = np.array_equiv(self.prev_map_grid, self.cur_map_grid)
        self.prev_map_grid = self.cur_map_grid
        #print(str(self.grid_x), str(self.grid_y))
        print("")

        return has_map_changed, self.cur_map_grid
    
    def dummy(self):
        pass