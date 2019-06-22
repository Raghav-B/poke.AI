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

    def __init__(self, w, h, pad, dft=4):
        self.window_width = w
        self.window_height = h
        self.padding = pad
        self.tile_size = int(w / (self.grid_x - 1))
        self.definite_frames_thresh = dft
        self.prev_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), ['.'], dtype=str)

    # bounding_box_list is of shape (box_dimensions, score, label)
    def convert_points_to_grid(self, bounding_box_list):
        # Converts real-world coordinates to grid values used in-game

        tiles = []
        for label, box in bounding_box_list:
            # Converting top left

            coords = [0, 0, 0, 0]

            print(box)
            #print(label)
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
    
            tiles.append((label, coords))
        return tiles

    def fill_area(self, area_bound, symbol):
        if (symbol == 'A'):
            self.cur_map_grid[area_bound[3]][area_bound[2]] = 'A'
        elif (symbol == 'X'):
            if (self.cur_map_grid[area_bound[1]][area_bound[0]] != 'A'):
                self.cur_map_grid[area_bound[1]][area_bound[0]] = 'X'
            if (self.cur_map_grid[area_bound[1]][area_bound[2]] != 'A'):
                self.cur_map_grid[area_bound[1]][area_bound[2]] = 'X'
        else:
            #print(area_bound)
            x = area_bound[0] 
            while (x <= area_bound[2]):
                y = area_bound[1]
                while (y <= area_bound[3]):
                    if (self.cur_map_grid[y][x] != 'A'):
                        self.cur_map_grid[y][x] = symbol
                    y += 1
                x += 1

    # Returns true if a change has been detected, otherwise returns false.
    def draw_map(self, bounding_box_list):
        tiles = self.convert_points_to_grid(bounding_box_list)
        print(tiles)
        symbol = None
        self.cur_map_grid = np.full((self.grid_y - 1, self.grid_x - 1), ['.'], dtype=str)
        has_map_changed = None

        for label, box in tiles:
            if (label == 0): # pokecen
                symbol = '@'
            elif (label == 1): # pokemart
                symbol = '$'
            elif (label == 2): # npc
                symbol = 'A'
            elif (label == 3): # house
                symbol = '#'
            elif (label == 4): # gym
                symbol = 'Y'
            elif (label == 5): # exit
                symbol = 'X'    
            self.fill_area(box, symbol)
        
        has_map_changed = np.array_equal(self.prev_map_grid, self.cur_map_grid)
        self.prev_map_grid = self.cur_map_grid
        print(self.cur_map_grid)
        print("")
        return has_map_changed

        #output_map = self.map_grid.view(np.uint8)
        #plt.matshow(output_map)
        #plt.pause(0.001)
        #plt.show()


"""
Predicates ???
- I get the number of midpoints in the window every frame.
- Index of midpoint will stay the same for the same object while it is on screen, however,
    once it is off screen and back on again, there is no guarantee it will retain the same index
- I should detect a particular midpoint index for a particular amount of time (frames) before I can use it to 
    draw my map. 
- Dimensions of game world is currently 720x480, with each tile being 48x48, this means that in terms of tiles the
    dimensions are: 15x10 tiles.
- THere is a y offset of +120 on my input images because of the padding, so remember to remove this.

Tile conversion algorithm:
- random point: (43, 293) > (43, 174.5) > 
    bot_left to tile
        x: (orig / (width)) * 16 n.xxxxx Take n + 1 as tile coord
        y: orig -= padding, dec = (orig / (height - padding*2)) * 11, n.xxxx, take the n - 1 only as the tile coord.
    bot_right to tile
        x: Take n-1
        y: Take n as the tile coord - 1
    top_left to tile
        x: Take n + 1
        y: Take n + 1
    top_right to tile
        x: Take n - 1
        y: Take n + 1
    5 < 24, therefore converted to 
"""





