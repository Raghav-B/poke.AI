import numpy as np
import math

from ram_searcher import ram_searcher
from path_finder import path_finder

# Think of this object as something akin to SLAM. This will basically simultaenously
# localize the player character and map out its surroundings. This live map will be
# used by the neural network that will control the player character
class live_map:
    # Variables to initialize
    window_width = 0
    window_height = 0 
    padding = 0 # Black bars used to make input square
    tile_size = 0 # real-world size of square tiles

    # Stores time that has passed since beginning of movement
    frametime = 1 # Initial value is 1

    # Internals used by mapper
    prev_map_grid = None # The detected map represented as a 2D array
    cur_map_grid = None
    #score_grid = None # Basically a duplicate of map_grid, but stores score of tiles instead
    # Num of tiles in x and y axies (+ 1) - the num of tiles is 1-indexed here btw
    grid_x = 16
    grid_y = 12
    # Coordinates of top_left game view tile in relation to starting point in global map
    map_offset_x = 0 
    map_offset_y = 0
    # Coordinates of top_left and bot_right tiles for global map, relative to starting point
    map_min_offset_x = 0
    map_max_offset_x = 14
    map_min_offset_y = 0
    map_max_offset_y = 10
    # List of all detected in terms of their global coordinates
    object_list = []
    # List of tiles that are actually walls/boundaries
    boundary_points = []

    # Variables for ram_searcher.py
    ram_search = None
    prev_pos = None
    cur_pos = None

    # Path finder object
    pf = None


    def __init__(self, w, h, pad):#, pid, xpa, ypa):
        self.window_width = w
        self.window_height = h
        self.padding = pad
        self.tile_size = int(w / (self.grid_x - 1))
        self.prev_map_grid = np.full((self.grid_y - 1, self.grid_x - 1, 4), [0, 0, 0, 0], dtype=np.uint8)
        self.cur_map_grid = np.full((self.grid_y - 1, self.grid_x - 1, 4), [0, 0, 0, 0], dtype=np.uint8)
        #self.prev_map_grid[:,:,1:] = 0 # Setting as unvisited
        #self.score_grid = np.full_like(self.prev_map_grid, 1)
        # Drawing character's position on map for localization purposes
        #self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 7] = 24
        
        # Setting up ram searcher
        self.ram_search = ram_searcher()
        self.prev_pos = self.ram_search.get_vals() # Storing character's position

        # Setting up path finder
        self.pf = path_finder()


    # Not the fastest function, is essentially a O(n^2) solution that fills in
    # the tiles covered by detected objects
    def fill_area(self, area_bound, symbol):
        if (np.array_equal(symbol, [66, 135, 245])): # npc
            self.cur_map_grid[area_bound[3]][area_bound[2]][:3] = [66, 135, 245]
        
        elif (np.array_equal(symbol, [105, 105, 105])): # boundary
            self.cur_map_grid[area_bound[3]][area_bound[2]][:3] = [105, 105, 105]
        
        elif (np.array_equal(symbol, [33, 255, 185])): # exit
            if (not np.array_equal(self.cur_map_grid[area_bound[1]][area_bound[0]][:3], [66, 135, 245])):
                self.cur_map_grid[area_bound[1]][area_bound[0]][:3] = [33, 255, 185]
            if (not np.array_equal(self.cur_map_grid[area_bound[1]][area_bound[2]][:3], [66, 135, 245])):
                self.cur_map_grid[area_bound[1]][area_bound[2]][:3] = [33, 255, 185]
        
        else:
            x = area_bound[0] 
            while (x <= area_bound[2]):
                y = area_bound[1]
                while (y <= area_bound[3]):
                    if (not np.array_equal(self.cur_map_grid[y][x][:3], [66, 135, 245])):
                        self.cur_map_grid[y][x][:3] = symbol
                    y += 1
                x += 1


    # Output of inferencing on each input frame is in terms of the frame's pixel coordinates, for our
    # use case, we have to convert these to in-game tiles
    def convert_points_to_grid(self, key_pressed, bounding_box_list):
        tiles = []
        for label, box in bounding_box_list:
            coords = [0, 0, 0, 0]

            # Object top_left corner
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

            # Object bot_right corner
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

            # Converting tiles in terms of local coordinates to global coordinates of entire map
            coords[0] += (self.map_offset_x - self.map_min_offset_x)
            coords[1] += (self.map_offset_y - self.map_min_offset_y)
            coords[2] += (self.map_offset_x - self.map_min_offset_x)
            coords[3] += (self.map_offset_y - self.map_min_offset_y)

            tiles.append((label, coords))
        
        return tiles
    
    def append_handler(self, key_pressed):
        is_appending = False # Flag for when player character is moving into unmapped regions

        # Conditionals below append to the map if the game view tries to pass an edge
        if (key_pressed == 0):
            # If game view tries to shift above global map edge
            if (self.map_offset_y - 1 < self.map_min_offset_y):
                self.grid_y += 1
                self.map_min_offset_y -= 1
                append_arr = np.full((1, self.grid_x - 1, 4), [0, 0, 0, 0], dtype=np.uint8)
                #append_arr[:,:,1:] = 0
                self.cur_map_grid = np.append(append_arr, self.cur_map_grid, axis=0)
                self.map_offset_y -= 1

                is_appending = True
            else:
                self.map_offset_y -= 1
        
        elif (key_pressed == 1):
            # If game view tries to shift right of global map edge
            if (self.map_offset_x + 1 + 14 > self.map_max_offset_x):
                self.grid_x += 1
                self.map_max_offset_x += 1
                append_arr = np.full((self.grid_y - 1, 1, 4), [0, 0, 0, 0], dtype=np.uint8)
                #append_arr[:,:,1:] = 0
                self.cur_map_grid = np.append(self.cur_map_grid, append_arr, axis=1)
                self.map_offset_x += 1

                is_appending = True
            else:
                self.map_offset_x += 1
            
        elif (key_pressed == 2):
            # If game view tries to shift below global map edge
            if (self.map_offset_y + 1 + 10 > self.map_max_offset_y):
                self.grid_y += 1
                self.map_max_offset_y += 1
                append_arr = np.full((1, self.grid_x - 1, 4), [0, 0, 0, 0], dtype=np.uint8)
                #append_arr[:,:,1:] = 0
                self.cur_map_grid = np.append(self.cur_map_grid, append_arr, axis=0)
                self.map_offset_y += 1

                is_appending = True
            else:
                self.map_offset_y += 1
            
        elif (key_pressed == 3):
            # If game view tries to shift to left of global map edge
            if (self.map_offset_x - 1 < self.map_min_offset_x):
                self.grid_x += 1
                self.map_min_offset_x -= 1
                append_arr = np.full((self.grid_y - 1, 1, 4), [0, 0, 0, 0], dtype=np.uint8)
                #append_arr[:,:,1:] = 0
                self.cur_map_grid = np.append(append_arr, self.cur_map_grid, axis=1)
                self.map_offset_x -= 1

                is_appending = True
            else:
                self.map_offset_x -= 1

        # If key_pressed is 'x' or None, do nothing in this case
        else:
            pass

        # Adjusting global coordinates if map is being appended in any of the 4 directions
        # In other words, if player character is stepping into unmapped territory. This is
        # important because all the objects' coordinates will change relative to a map that
        # increases in size
        if (is_appending == True):
            # Yes there is a reason why "right" and "down" are blank
            for label, box in self.object_list:
                if (key_pressed == 0):
                    box[1] += 1
                    box[3] += 1
                elif (key_pressed == 1):
                    pass
                elif (key_pressed == 2):
                    pass
                elif (key_pressed == 3):
                    box[0] += 1
                    box[2] += 1
                else:
                    pass

    # This function handles detection of new objects and uses this new information to further built
    # the global map
    def add_to_object_list(self, key_pressed, bounding_box_list):
        # Reset map to blank slate as a failsafe against other functions that might access this map
        #self.cur_map_grid = np.full((self.grid_y - 1, self.grid_x - 1, 4), [0, 0, 0, 0], dtype=np.uint8)
        #self.cur_map_grid[:,:,1:] = self.prev_map_grid[:,:,1:]
        self.cur_map_grid[:,:,:3] = [0, 0, 0]

        # Function to handle changes in global coordinates if map_grid needs to be appended to
        self.append_handler(key_pressed)

        # Get newly detected objects in terms of tiles
        tiles = self.convert_points_to_grid(key_pressed, bounding_box_list)

        # O(n^2) solution for keeping track of new and old objects. Perhaps a faster method exists?
        # This basically gets the latest detected objects and checks whether they are in fact the same
        # as previously detected objects or are completely new objects
        temp_object_list = [] # Temp list to store newly detected objects
        for new_label, new_box in tiles: # Iterating through objects detected this frame
            is_found = False

            for label, box in self.object_list: # Iterating through previously detected objects
                # This conditional checks whether a new object is the same as an old object by checking if the top_left or
                # bot_right points reside inside an old object's area
 
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

                    # The so-called newly detected object is in-fact an old object, we can safely break from this loop
                    is_found = True
                    break

            # If object is not found, i.e., it is a new object, prepare it for adding to the main object_list
            if (is_found == False):
                temp_object_list.append((new_label, new_box))
        
        # Add newly found objects to list
        self.object_list.extend(temp_object_list)


    # This is called from main.py to draw our global map. Inputs are the bounding boxes raw data from
    # the frame inferencing and the most recent key pressed by the controller
    def draw_map(self, key_pressed, bounding_box_list):
        self.cur_pos = self.ram_search.get_vals() # Player position from ram searcher
        has_map_changed = True # Flag for wall/collision detection
        dir_to_move = None

        self.boundary_points = []
        # These conditionals handle the wall/collision detection by comparing the previous player position with the new
        # player position based on the most recent key press
        if (key_pressed == 0):
            if (self.cur_pos[1] == self.prev_pos[1]):
                has_map_changed = False
                dir_to_move = np.random.choice([1,2,3])
                if (np.array_equal(self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 4][(self.map_offset_x - self.map_min_offset_x) + 7][:3], [255, 255, 255])):
                    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 7, (self.map_offset_y - self.map_min_offset_y) + 4))
        
        elif (key_pressed == 1):
            if (self.cur_pos[0] == self.prev_pos[0]):
                has_map_changed = False
                dir_to_move = np.random.choice([0,2,3])
                if (np.array_equal(self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 8][:3], [255, 255, 255])):
                    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 8, (self.map_offset_y - self.map_min_offset_y) + 5))
        
        elif (key_pressed == 2):
            if (self.cur_pos[1] == self.prev_pos[1]):
                dir_to_move = np.random.choice([0,1,3])
                has_map_changed = False
                if (np.array_equal(self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 6][(self.map_offset_x - self.map_min_offset_x) + 7][:3], [255, 255, 255])):
                    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 7, (self.map_offset_y - self.map_min_offset_y) + 6))
        
        elif (key_pressed == 3):
            if (self.cur_pos[0] == self.prev_pos[0]):
                dir_to_move = np.random.choice([0,1,2])
                has_map_changed = False
                if (np.array_equal(self.prev_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 6][:3], [255, 255, 255])):
                    self.boundary_points.append(((self.map_offset_x - self.map_min_offset_x) + 6, (self.map_offset_y - self.map_min_offset_y) + 5))
        else:
            pass

        # If collision has been detected, we return from this function otherwise the map will be
        # incorrectly drawn and this will mess everything up
        if (has_map_changed == False):
            print("COLLISION!")
            
            for point in self.boundary_points:
                coords = [point[0], point[1], point[0], point[1]]
                #coords[0] = point[0] #+ (self.map_offset_x - self.map_min_offset_x)
                #coords[1] = point[1] #+ (self.map_offset_y - self.map_min_offset_y)
                #coords[2] = point[0] #+ (self.map_offset_x - self.map_min_offset_x)
                #coords[3] = point[1] #+ (self.map_offset_y - self.map_min_offset_y)
                if not ((6, coords) in self.object_list):
                    self.object_list.append((6, coords))

                self.fill_area(coords, [105, 105, 105])

            # Used for anything that needs to compare previous map state with new map state
            self.prev_map_grid = self.cur_map_grid
            self.prev_pos = self.cur_pos
            
            # Put path finder function here
            dir_penalties = [0, 0, 0, 0]
            col_penalty = (key_pressed, 5)
            self.cur_map_grid = self.pf.get_next_dir(self.cur_map_grid, self.frametime, \
                (self.map_offset_x - self.map_min_offset_x), (self.map_offset_y - self.map_min_offset_y), \
                col_penalty=col_penalty)
            print("Penalty given")

            self.frametime += 1
            #print(self.object_list)
            # If collision is detected, move in random direction except our last direction
            return self.cur_map_grid, 0#dir_to_move
        
        # Use bounding box list to add to our list of global objects
        self.add_to_object_list(key_pressed, bounding_box_list)
        #print(self.object_list)

        # This block handles drawing of tiles in the map with different colours on the grayscale spectrum
        symbol = None
        for label, box in self.object_list:
            if (label == 0): # pokecen
                symbol = [0, 0, 255] # red
            elif (label == 1): # pokemart
                symbol = [255, 0, 0] # blue
            elif (label == 2): # npc
                symbol = [66, 135, 245] # orange
            elif (label == 3): # house
                symbol = [30, 57, 102] # brown
            elif (label == 4): # gym
                symbol = [96, 102, 30] # turqoise
            elif (label == 5): # exit
                symbol = [33, 255, 185] # yellow
            elif (label == 6): # wall/boundary
                symbol = [105, 105, 105] # grey
            self.fill_area(box, symbol)
        # Draw player character position for localization purpose # green
        self.cur_map_grid[(self.map_offset_y - self.map_min_offset_y) + 5][(self.map_offset_x - self.map_min_offset_x) + 7][:3] = [33, 166, 28]

        # Put path finder function here
        self.cur_map_grid = self.pf.get_next_dir(self.cur_map_grid, self.frametime, \
            (self.map_offset_x - self.map_min_offset_x), (self.map_offset_y - self.map_min_offset_y))

        # Used for anything that needs to compare previous map state with new map state
        self.prev_map_grid = self.cur_map_grid
        self.prev_pos = self.cur_pos

        # Time is only updated after score is calculated
        self.frametime += 1

        return self.cur_map_grid, 0#dir_to_move