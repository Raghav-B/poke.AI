"""
Coming up with a custom reward function is hard :(

Things I can do:
- Add more classes into game. This can help more with mapping and seeing ways to explore the map
    - Use some kind of BFS/DFS to find a way to get into open spaces in the map.

- How does a human do it? A human will look for open spaces where there are no boundaries.
    - 

What I want the behaviour to be like:
- Over time, I want the agent to be able to explore the entire map, even if it means by using a pseudo-brute force exploration method.
- If the agent spends too long in a certain area, it should be told to move in a relatively less explored region.
    - I can get a measure of how well-explored a region is by taking the count of how many times a particular tile has been in our gameplay frame.
    - If a tile has appeared say, 10 times, then we have explored this area too much. At this point we move to a point where the tile appearance is maybe 1. This should be somewhere along the edge of our map. We use a measure of average tile appearance. If the average of our region is above maybe 10 or something, we find the closest tile that is 1 using a BFS. 
    - This BFS will return us a sequence of steps we have to take to get to this tile. If, in the process of getting to this tile, we are interrupted by an unseen collision (this has to be an unseen collision because otherwise our algorithm wouldn't give us this sequence of steps), we mark this collision, and use a mini BFS to find the fastest way to get to our next point in the sequence. 
        - If we have 5 consequent collisions on our way to this point, we ignore this point and try to find  
    - In this step, we ignore all scores, but keep the score algo running and simply move to our chosen location. Should we implment a BFS for this? Perhaps, this will give us the path to the closest map_edge. 

- Using Deep Q Learning or something:
    - We gave 4 actions. Move up, down, left or right. 
    - 


The less your offset has changed from the starting offset in a particular direction, the higher chance there is of you exploring other offsets. 

rand_thresh = 100 - (abs(x_offset_change - y_offset_change))
x = random.randint(0, 100)
if (x > rand_thresh):
    #go in particular direction


We can use a frontier based exploration algorithm, where frontiers are determined in terms of their open space.
Frontier points will be any points that are on the edge of our current grid_map. Points that have come inside the frame are considered detected, while those not on our frame are considered undetected.

To find frontier points, we basically do a breadth first search of all unvisited edges of the map, and calculate a score for each potential frontier point. We can store these inside a priority queue so that we only retrieve the point that is bound to give us the highest potential score.

This potential score measure is based on a measure of proximity to buildings, open space (potential for exploration) and. 

Start of BFS for frontier detection:
    - We can start the BFS from the top left corner of the frame.
    - 

How will a frontier be scored?
    - Based on the values of the tiles on 4 or 8 of its adjacent sides. Certain objects give higher values. 
    - Black, unvisited tiles give the highest value as these have the greatest potential for a new object or score. 
    - White, visited tiles give the lowest value.
    - Houses, gyms, other buildings give varying values of score to a frontier. 


Moving towards a frontier:
    - A better thing to do might be to run a BFS to find the shortest path to the frontier. 
    - We find the difference in y and x separately from our current pos to the pos of the frontier. Based on a random coin flip, we will either start in the y direction first or in the x direction first.
    - This part of the algorithm will use the pledge algorithm as a means to move around obstacles and to get to our final destination.
    - If we are unable to get to our final destination after some amount of time or collisions, we will mark this frontier as unreachable and will continue to another frontier. 
    - In the process of moving around obstacles while trying to get to a frontier, we will eventually learn more about the world as more of it will be mapped out. 
    - If 5 consecutive collisions are detected when trying to reach for a frontier, we will consider that frontier unreachable, and will choose another frontier.

Colliding with an object when trying to reach a frontier:
- If this happens, we have a 5 consecutive collision counter to ensure that our frontier isn't unreachable. 
- To move past a collision, we will run a BFS from our current point to the next to next position we're supposed to be at. 

Upon reaching a frontier:
    - Can be an arbitrary limit such as every 3 houses, we stop chasing frontiers, I'm not sure. 
    - Once we reach a frontier and a new unvisited object has been detected, we will go to the object. Path to take to object can be done using a simple BFS. 
    - If no new objects are detected, we will scrap previous frontiers and run a BFS again to detect more frontiers. 

BFS:
    - This will output a list of moves to make to reach a frontier. Its better to output a list of moves rather than a list of coordinates because these coordinates can get very messy when it comes to shifting from global to local coordinates and whatnot.
"""

# score_grid has a base score of 1 for each tile

import numpy as np
import random
import heapq as pq

class path_finder:
    #score_grid = None
    # some sort of start_time variable???
    local_top_x = 0
    local_top_y = 0
    local_bot_x = 0
    local_bot_y = 0

    frontier_list = []

    #map_grid = None

    def __init__(self):
        self.frontier_list 
        pass

    def get_frontier_score(self, query_pos):
        if (np.array_equal(query_pos[:3], [0, 0, 0])): # Unvisited
            return 100
        elif (np.array_equal(query_pos[:3], [255, 255, 255])): # Visited
            return 0
        elif (np.array_equal(query_pos[:3], [96, 102, 30])): # Gym
            return 85
        elif (np.array_equal(query_pos[:3], [30, 57, 102])): # House
            return 70
        elif (np.array_equal(query_pos[:3], [0, 0, 255])): # Pokecen
            return 55
        elif (np.array_equal(query_pos[:3], [255, 0, 0])): # Pokemart
            return 40
        elif (np.array_equal(query_pos[:3], [105, 105, 105])): # Wall/Boundary
            return -20

    def find_frontier_bfs(self, map_grid, cur_pos, end_pos, move_list, ):
        frontier_breadth_list = []
        score = 0

        # Marking the current point as visited by our BFS. Visited = 3 (just an arbitrary number lol)
        map_grid[cur_pos[1]][cur_pos[0]][3] = 3

        # Up direction
        if (cur_pos[1] - 1 >= 0): # Checking if inside 2d array
            score += self.get_frontier_score(map_grid[cur_pos[1] - 1][cur_pos[0]])
            # Check if unvisited by DFS
            if (map_grid[cur_pos[1] - 1][cur_pos[0]][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [0, 0, 0])):
                    frontier_breadth_list.append((cur_pos[0], cur_pos[1] - 1))
        
        # Right direction
        if (cur_pos[0] + 1 <= map_grid.shape[1] - 1): # CHecking if inside 2d array
            score += self.get_frontier_score(map_grid[cur_pos[1]][cur_pos[0] + 1])
            # Check if unvisited by DFS
            if (map_grid[cur_pos[1]][cur_pos[0] + 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [0, 0, 0])):
                    frontier_breadth_list.append((cur_pos[0] + 1, cur_pos[1]))

        # Down direction
        if (cur_pos[1] + 1 <= map_grid.shape[0] - 1): # CHecking 
            score += self.get_frontier_score(map_grid[cur_pos[1]][cur_pos[0] + 1])
            # Check if unvisited by DFS
            if (map_grid[cur_pos[1]][cur_pos[0] + 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [0, 0, 0])):
                    frontier_breadth_list.append((cur_pos[0] + 1, cur_pos[1]))

        # Left direction
        if (cur_pos[0] - 1 >= 0): # Checking
            pass

        # Top left

        # Top right

        # Bottom left

        # Bottom right
        




        return move_list

    def move_to_frontier_bfs(self, start_pos, end_pos, move_list, ):

        pass


    def calc_score(self, tile, cur_frametime, local_decay_multiplier):
        tile_score = 0

        # Conditionals check if tile is of a particular type
        #if (tile[0] == 255): # normal tile
        tile_score = 5 * (1 / (((cur_frametime - tile[3]) * local_decay_multiplier) + 1))
        #elif (tile[0] == 16): # pokecen
        #    tile_score = 30 * (1 / (((cur_frametime - tile[1]) * local_decay_multiplier) + 1))
        #elif (tile[0] == 32): # pokemart
        #    tile_score = 20 * (1 / (((cur_frametime - tile[1]) * local_decay_multiplier) + 1))
        #elif (tile[0] == 0): # npc # Unused for now also
        
        #elif (tile[0] == 64): # house
        #    tile_score = 50 * (1 / (((cur_frametime - tile[1]) * local_decay_multiplier) + 1))
        #elif (tile[0] == 48): # gym
        #    tile_score = 100 * (1 / (((cur_frametime - tile[1]) * local_decay_multiplier) + 1))
        # Unsed for now
        #elif (tile[128] == 128): # exit
        
        return tile_score

    def get_next_dir(self, map_grid, cur_frametime, top_x, top_y, col_penalty=(0, 0)):        
        #center_x = int(map_grid.shape[1] / 2)
        #center_y = int(map_grid.shape[0] / 2)

        col_penalties = [0, 0, 0, 0]
        col_penalties[col_penalty[0]] = col_penalty[1]

        center_x = top_x + 7
        center_y = top_y + 5

        #print(center_x, center_y)

        self.local_top_x = top_x
        self.local_top_y = top_y
        self.local_bot_x = top_x + 14
        self.local_bot_y = top_y + 10

        #print(self.local_top_x, self.local_top_y)
        #print(self.local_bot_x, self.local_bot_y)
        
        for i in range(0, len(map_grid)):
            for j in range(0, len(map_grid[i])):
                if (j > self.local_top_x and j < self.local_bot_x) and \
                    (i > self.local_top_y and i < self.local_bot_y):
                    map_grid[i][j][3] = 1
                    if (np.array_equal(map_grid[i][j][:3], [0, 0, 0])):
                        map_grid[i][j] = [255, 255, 255, 1]
                elif (map_grid[i][j][3] == 1):
                    if (np.array_equal(map_grid[i][j][:3], [0, 0, 0])):
                        map_grid[i][j][:3] = [255, 255, 255]



        # for i in range(self.local_top_y + 1, self.local_bot_y):
        #for j in range(self.local_top_x + 1, self.local_bot_x):
        #   if (np.array_equal(map_grid[i][j][:3], [0, 0, 0])):
        #       map_grid[i][j] = [255, 255, 255, 1]

        return map_grid
        """
        top_left_score = 0
        for row in range(0, len(map_grid[:center_y + 1])):
            for col in range(0, len(map_grid[row][:center_x + 1])):
                if (col >= self.local_top_x and col <= self.local_bot_x) and \
                    (row >= self.local_top_y and row <= self.local_bot_y):
                    top_left_score += (self.calc_score(map_grid[row][col], cur_frametime, 1) - col_penalties[0])
                #else:
                    #top_left_score += self.calc_score(map_grid[row][col], cur_frametime, 1)
        
        #for row in map_grid[:center_y + 1, :center_x + 1]:
        #    for tile in row:
        #        top_left_score += self.calc_score(tile, cur_frametime, 1)

        top_right_score = 0
        for row in range(0, len(map_grid[:center_y + 1])):
            for col in range(center_x, len(map_grid[row])):
                if (col >= self.local_top_x and col <= self.local_bot_x) and \
                    (row >= self.local_top_y and row <= self.local_bot_y):
                    top_right_score += (self.calc_score(map_grid[row][col], cur_frametime, 1) - col_penalties[1])
                #else:
                    #top_right_score += self.calc_score(map_grid[row][col], cur_frametime, 1)
        
        #for row in map_grid[:center_y + 1, center_x:]:
        #    for tile in row:
        #        top_right_score += self.calc_score(tile, cur_frametime, 1)
        
        bot_left_score = 0
        for row in range(center_y, len(map_grid)):
            for col in range(0, len(map_grid[row][:center_x + 1])):
                if (col >= self.local_top_x and col <= self.local_bot_x) and \
                    (row >= self.local_top_y and row <= self.local_bot_y):
                    bot_left_score += (self.calc_score(map_grid[row][col], cur_frametime, 1) - col_penalties[2])
                #else:
                    #bot_left_score += self.calc_score(map_grid[row][col], cur_frametime, 1)
        
        #for row in map_grid[center_y:, :center_x + 1]:
        #    for tile in row:
        #        bot_left_score += self.calc_score(tile, cur_frametime, 1)
        
        bot_right_score = 0
        for row in range(center_y, len(map_grid)):
            for col in range(center_x, len(map_grid[row])):
                if (col >= self.local_top_x and col <= self.local_bot_x) and \
                    (row >= self.local_top_y and row <= self.local_bot_y):
                    bot_right_score += (self.calc_score(map_grid[row][col], cur_frametime, 1) - col_penalties[3])
                #else:
                    #bot_right_score += self.calc_score(map_grid[row][col], cur_frametime, 1)

        #for row in map_grid[center_y:, center_x:]:
        #    for tile in row:
        #        bot_right_score += self.calc_score(tile, cur_frametime, 1)
            
        scores = [(top_left_score + top_right_score), (top_right_score + bot_right_score), \
            (bot_right_score + bot_left_score), (bot_left_score + top_left_score)]
        print(f"up: {scores[0]}")
        print(f"right: {scores[1]}")
        print(f"down: {scores[2]}")
        print(f"left: {scores[3]}")

        max_score = max(scores)
        min_score = min(scores)
        if (abs(max_score - min_score) <= 10):
            print("Very similar, taking random dir")
            return random.randint(0, 3)
        else:
            return scores.index(max_score) # Returns the best movement to make based on score
        """

        return 0