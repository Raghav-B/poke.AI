import matplotlib.pyplot as plt
import numpy as np
import math

from ram_searcher import ram_searcher

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
    grid_x = 16 # Tiles in x axis + 1, num of tiles is 1 indexed
    grid_y = 12 # Tiles in y axis + 1

    object_list = []
    boundary_points = []

    map_offset_x = 0
    map_offset_y = 0
    map_min_offset_x = 0
    map_max_offset_x = 14
    map_min_offset_y = 0
    map_max_offset_y = 10

    ram_search = None
    prev_pos = None
    cur_pos = None

    def __init__(self, w, h, pad, pid, xpa, ypa, dft=4):
        self.window_width = w
        self.window_height = h
        self.padding = pad
        self.tile_size = int(w / (self.grid_x - 1))
        self.definite_frames_thresh = dft
        self.prev_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), 255, dtype=np.uint8)
        self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 7] = 24
        
        self.ram_search = ram_searcher(pid, xpa, ypa)
        #self.prev_pos = [self.ram_search.get_x_pos(), self.ram_search.get_y_pos()]

    # bounding_box_list is of shape (label, box_dimensions)
    def convert_points_to_grid(self, is_appending, key_pressed, bounding_box_list):
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

            coords[0] += (self.map_offset_x - self.map_min_offset_x)
            coords[1] += (self.map_offset_y - self.map_min_offset_y)
            coords[2] += (self.map_offset_x - self.map_min_offset_x)
            coords[3] += (self.map_offset_y - self.map_min_offset_y)

            """
            if (key_pressed == "up"):
                if (is_appending == False):
                    coords[1] += self.map_offset_y
                    coords[3] += self.map_offset_y
            elif (key_pressed == "left"):
                if (is_appending == False):
                    coords[0] += self.map_offset_x
                    coords[2] += self.map_offset_x
            elif (key_pressed == "right"):
                coords[0] += self.map_offset_x
                coords[2] += self.map_offset_x
            elif (key_pressed == "down"):
                coords[1] += self.map_offset_y
                coords[3] += self.map_offset_y
            else:
                pass
            """

            tiles.append((label, coords))
        return tiles # This returns the global converted version of the detected objects

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
        is_appending = False
        self.cur_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), 255, dtype=np.uint8)
        if (key_pressed == "up"):
            if (self.map_offset_y - 1 < self.map_min_offset_y):
                self.grid_y += 1
                self.map_min_offset_y -= 1
                append_arr = np.full((1, self.grid_x - 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(append_arr, self.cur_map_grid, axis=0)
                self.map_offset_y -= 1

                is_appending = True
            else:
                self.map_offset_y -= 1
        
        elif (key_pressed == "right"):
            if (self.map_offset_x + 1 + 14 > self.map_max_offset_x):
                self.grid_x += 1
                self.map_max_offset_x += 1
                append_arr = np.full((self.grid_y - 1, 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(self.cur_map_grid, append_arr, axis=1)
                self.map_offset_x += 1

                is_appending = True
            else:
                self.map_offset_x += 1
            
        elif (key_pressed == "down"):
            if (self.map_offset_y + 1 + 10 > self.map_max_offset_y):
                self.grid_y += 1
                self.map_max_offset_y += 1
                append_arr = np.full((1, self.grid_x - 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(self.cur_map_grid, append_arr, axis=0)
                self.map_offset_y += 1

                is_appending = True
            else:
                self.map_offset_y += 1
            
        elif (key_pressed == "left"):
            if (self.map_offset_x - 1 < self.map_min_offset_x):
                self.grid_x += 1
                self.map_min_offset_x -= 1
                append_arr = np.full((self.grid_y - 1, 1), 255, dtype=np.uint8)
                self.cur_map_grid = np.append(append_arr, self.cur_map_grid, axis=1)
                self.map_offset_x -= 1

                is_appending = True
            else:
                self.map_offset_x -= 1

        else: # If key is "x"
            pass

        tiles = self.convert_points_to_grid(is_appending, key_pressed, bounding_box_list)
        """
        for point in self.boundary_points:
            #if (self.prev_map_grid[point[1]][point[0]] == 255):
            coords = [0, 0, 0, 0]
            coords[0] = point[0] + (self.map_offset_x - self.map_min_offset_x)
            coords[1] = point[1] + (self.map_offset_y - self.map_min_offset_y)
            coords[2] = point[0] + (self.map_offset_x - self.map_min_offset_x)
            coords[3] = point[1] + (self.map_offset_y - self.map_min_offset_y)
            tiles.append((6, coords))
        """

        # Adjusting global coordinates if map is being appended in any of the 4 directions
        if (is_appending == True):
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
                if (new_box[0] >= box[0] and new_box[0] <= box[2] and new_box[1] >= box[1] and new_box[1] <= box[3]) or \
                    (new_box[2] >= box[0] and new_box[2] <= box[2] and new_box[3] >= box[1] and new_box[3] <= box[3]):
                    # If this is true then the two objects are indeed the same, now we need to decide whether we keep
                    # the old object or the new one. This is based on the area (size) of the object. We keep the one
                    # with the larger area.
                    new_area = (new_box[2] - new_box[0]) * (new_box[3] - new_box[1])
                    og_area = (box[2] - box[0]) * (box[3] - box[1])

                    if (new_area > og_area):
                        box[:] = new_box[:]
                    # Else we keep the original object as it is

                    is_found = True
                    break
                
                #if ((new_box[0] == box[0] and new_box[1] == box[1]) or \
                #    (new_box[2] == box[2] and new_box[3] == box[3])):
                #    if (abs(new_box[0] - new_box[2]) > abs(box[0] - box[2]) or \
                #        abs(new_box[1] - new_box[3]) > abs(box[1] - box[3])):
                #        box[:] = new_box[:]
                #    is_found = True
                #    break
            
            if (is_found == False):
                temp_object_list.append((new_label, new_box))
        
        self.object_list.extend(temp_object_list)
        #return tiles

    # Returns true if a change has been detected, otherwise returns false.
    def draw_map(self, key_pressed, bounding_box_list):
        self.cur_pos = [self.ram_search.get_x_pos(), self.ram_search.get_y_pos()]
        print(self.prev_pos)
        print(self.cur_pos)
        has_map_changed = True

        if (key_pressed == "up"):
            if (self.cur_pos[1] == self.prev_pos[1]):
                has_map_changed = False
                #if (self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 4][(self.map_offset_x - self.map_min_offset_x) + 7] == 255):
                #    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 7, (self.map_offset_y - self.map_min_offset_y) + 4))
        elif (key_pressed == "right"):
            if (self.cur_pos[0] == self.prev_pos[0]):
                has_map_changed = False
                #if (self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 8] == 255):
                #    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 8, (self.map_offset_y - self.map_min_offset_y) + 5))
        elif (key_pressed == "down"):
            if (self.cur_pos[1] == self.prev_pos[1]):
                has_map_changed = False
                #if (self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 6][(self.map_offset_x - self.map_min_offset_x) + 7] == 255):
                #    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 7, (self.map_offset_y - self.map_min_offset_y) + 6))
        elif (key_pressed == "left"):
            if (self.cur_pos[0] == self.prev_pos[0]):
                has_map_changed = False
                #if (self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 6] == 255):
                #    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 6, (self.map_offset_y - self.map_min_offset_y) + 5))
        else:
            pass

        #for point in self.boundary_points:
            #if (self.prev_map_grid[point[1]][point[0]] == 255):
            #self.cur_map_grid[point[1]][point[0]] = 200

        if (has_map_changed == False):
            print("collision!")
            #self.prev_map_grid = self.cur_map_grid
            return self.cur_map_grid
        
        self.add_to_object_list(key_pressed, bounding_box_list)
        #print(self.object_list)

        symbol = None
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
            #elif (label == 6): # wall
            #    symbol = 200
            self.fill_area(box, symbol)
        self.cur_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 7] = 24

        self.prev_map_grid = self.cur_map_grid
        self.prev_pos = self.cur_pos
        #print(str(self.grid_x), str(self.grid_y))
        #print("")

        print(self.map_offset_x, self.map_offset_y)
        return self.cur_map_grid
    
    def draw_init_map(self, key_pressed, bounding_box_list):
        tiles = self.convert_points_to_grid(None, key_pressed, bounding_box_list)
        symbol = None
        self.cur_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), 255, dtype=np.uint8)
        #print(len(self.object_list))
        for label, box in tiles:
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
            self.object_list.append((label, box))
        self.cur_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 7] = 24

        print(self.object_list)

        self.prev_map_grid = self.cur_map_grid
        return self.cur_map_grid
    
    def dummy(self):
        pass



"""
MAPPING ALGORITHM

Global coordinates:
If I'm moving up and appending, the global y coordinates of every object increases by 1
If I'm moving down and appending, the global y coordinates of every object remains the same

If I'm moving left and appending, the global x coordinaes of every object increases by 1
If I'm moving right and appending, the global x coordinates of every object remains the same

If I'm moving without appending, global coordinates remain the same.

Local coordinates: (changing from local to global)
If I'm moving up, decrease y-offset. Add this to every local coordinate to make it global
If I'm moving down, increase y-offset. Add this to every local coordinate to make it global

If I'm moving left, decrease x-offset. Add this to every local coordinate to make it global
If I'm moving right, increase y-offset. Add this to every local coordinate to make it global

Sorting objects:
When sorting objects, they are sorted based on their global coordinates. Therefore every new batch of local objects
has to be converted to global.

If a new local2global coordinate is found between another object's global coordinate space, we know that the two objects
are the same.



"""