"""
So how am I going to get this to work?

I need to send the map_grid here, and the score tally for each direction will be done right here.
But what's the most efficient way to do the score tallying?
- Initialise my map_grid as a 2D array of tuples, such that each tile comes hand in hand with a score associated with it.
    - This method is good for score decay calculation.
    - Problem is that I have to do some kind of custom print function to deal with this. In terms of space complexity this is exactly the same as the second method below. 
- Have a duplicate map_grid array which, instead of grayscale numbers, contains floats for the score for each tile.
    - This array would be created before/after every action, at the same time as cur_grid. Will this possibly create a slowdown?
    - So I only pass the score_cur_grid array to this script.

Getting score tally for each side:
- Best way to do this is to probably use python array splicing. 
- Calculating score for each corner, then for top down left right, do the following:
    - W: top_left + top_right
    - A: top_left + bot_left
    - S: bot_left + bot_right
    - D: bot_right + top_right
- This ensures that none of my score calculation is done twice, this is quite efficient.

Storage in this script:
- This stores the current score array
- Storing it here allows us to handle all score time decay in this script itself

Inputs to this script:
- cur grid array
- Time variables?
- midpoint coords (basically character position. This is used for splicing the array)

Outputs:
- Best direction to move in.
- Modified score grid array?
- Print scores in each direction for debugging

- Simple algo without time decay
for every tile in splice:
    if tile == something.....
        score based on tile = something....
    ...continue for each type of tile....
for top, bot, left, right:
    add splices to figure out highest score out of these.
return dir to move in.

NOTE: I NEED TO MODIFY MAPPER TO RETURN THE PREDICTED DIRECTION TO MOVE IN, THIS VARIABLE WILL THEN BE PASSED TO THE CTRL OBJECT TO TRIGGER THE NEXT APPROPRIATE MOVEMENT

One array of size map_grid that just stores time of detection of every object.

"""

# score_grid has a base score of 1 for each tile

import numpy as np

class path_finder:
    #score_grid = None
    # some sort of start_time variable???

    def __init__(self):
        pass

    def calc_score(self, tile, cur_frametime):
        tile_score = 0
        # Conditionals check if tile is of a particular type
        if (tile[0] == 255): # normal tile
            tile_score = 5 * (1 / ((cur_frametime - tile[1]) + 1))
        elif (tile[0] == 16): # pokecen
            tile_score = 30 * (1 / ((cur_frametime - tile[1]) + 1))
        elif (tile[0] == 32): # pokemart
            tile_score = 20 * (1 / ((cur_frametime - tile[1]) + 1))
        #elif (tile[0] == 0): # npc # Unused for now also
        
        elif (tile[0] == 64): # house
            tile_score = 50 * (1 / ((cur_frametime - tile[1]) + 1))
        elif (tile[0] == 48): # gym
            tile_score = 100 * (1 / ((cur_frametime - tile[1]) + 1))
        # Unused for now
        #elif (tile[128] == 128): # exit
        
        return tile_score

    def get_next_dir(self, map_grid, cur_frametime):        
        center_x = int(map_grid.shape[1] / 2)
        center_y = int(map_grid.shape[0] / 2)
        
        top_left_score = 0
        for row in map_grid[:center_y, :center_x]:
            for tile in row:
                top_left_score += self.calc_score(tile, cur_frametime)
        
        top_right_score = 0
        for row in map_grid[:center_y, center_x:]:
            for tile in row:
                top_right_score += self.calc_score(tile, cur_frametime)
        
        bot_left_score = 0
        for row in map_grid[center_y:, :center_x]:
            for tile in row:
                bot_left_score += self.calc_score(tile, cur_frametime)
        
        bot_right_score = 0
        for row in map_grid[center_y:, center_x:]:
            for tile in row:
                bot_right_score += self.calc_score(tile, cur_frametime)
            
        scores = [(top_left_score + top_right_score), (top_right_score + bot_right_score), \
            (bot_right_score + bot_left_score), (bot_left_score + top_left_score)]
        print(scores)

        return scores.index(max(scores)) # Returns the best movement to make based on score