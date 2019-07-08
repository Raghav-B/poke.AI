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

"""

class path_finder:
    score_grid = None
    # some sort of start_time variable???

    def __init__(self):
        pass

    def get_next_dir(self, map_grid, center_x, center_y):
        # This is where the score will be assigned to the score grid
        # Make score_grid be a copy of map_grid such that it has the same dimensions, but scores for every tile is initialized as zero.
        # Wait, do I really have to even store the score of. Yes I do, because when it comes to decay, this is absolutely necessary
        pass

    

