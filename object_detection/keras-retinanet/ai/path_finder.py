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

import numpy as np
import random
import heapq as pq

class path_finder:
    local_top_x = 0
    local_top_y = 0
    local_bot_x = 0
    local_bot_y = 0

    map_grid = None
    frontier_list = []
    next_frontier = None

    consecutive_collisions = 0

    def __init__(self):
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

    def find_frontier_bfs(self, cur_pos):
        frontier_breadth_list = []
        #can_continue = False
        score = 0

        # Up direction
        if (cur_pos[1] - 1 >= 0): # Checking if inside 2d array
            score += self.get_frontier_score(self.map_grid[cur_pos[1] - 1][cur_pos[0]])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] = 3
                    frontier_breadth_list.append([cur_pos[0], cur_pos[1] - 1])
                    #can_continue = True
        
        # Right direction
        if (cur_pos[0] + 1 <= self.map_grid.shape[1] - 1): # Checking if inside 2d array
            score += self.get_frontier_score(self.map_grid[cur_pos[1]][cur_pos[0] + 1])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] + 1, cur_pos[1]])
                    #can_continue = True

        # Down direction
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1): # Checking 
            score += self.get_frontier_score(self.map_grid[cur_pos[1] + 1][cur_pos[0]])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] = 3
                    frontier_breadth_list.append([cur_pos[0], cur_pos[1] + 1])
                    #can_continue = True

        # Left direction
        if (cur_pos[0] - 1 >= 0): # Checking
            score += self.get_frontier_score(self.map_grid[cur_pos[1]][cur_pos[0] - 1])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] - 1, cur_pos[1]])
                    #can_continue = True

        # Top left
        if (cur_pos[1] - 1 >= 0 and cur_pos[0] - 1 >= 0):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] - 1][cur_pos[0] - 1])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0] - 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] = 3
                    frontier_breadth_list.append([cur_pos[0] - 1, cur_pos[1] - 1])
                    #can_continue = True

        # Top right
        if (cur_pos[1] - 1 >= 0 and cur_pos[0] + 1 <= self.map_grid.shape[1] - 1):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] + 1, cur_pos[1] - 1])
                    #can_continue = True

        # Bottom left
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1 and cur_pos[0] - 1 >= 0):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] - 1, cur_pos[1] + 1])
                    #can_continue = True

        # Bottom right
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1 and cur_pos[0] + 1 <= self.map_grid.shape[1] - 1):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1])
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] + 1, cur_pos[1] + 1])
                    #can_continue = True

        # Pushing current frontier to priority queue
        pq.heappush(self.frontier_list, [-score, cur_pos[0], cur_pos[1]])

        return frontier_breadth_list
    
    def ffb_wrapper(self, cur_pos):
        points = [cur_pos]
        # Marking the current point as visited by our BFS. Visited = 3 (just an arbitrary number lol)
        self.map_grid[cur_pos[1]][cur_pos[0]][3] = 3
        while True:
            level_points = []
            for point in points:
                temp_points = self.find_frontier_bfs(point)
                level_points.extend(temp_points)
            
            if (len(level_points) == 0):
                return
            else:
                points = level_points

    def move_to_frontier_bfs(self, cur_pos, end_pos, move_list):
        if (cur_pos[:] == end_pos[:]):
            return True, [[cur_pos, move_list]]

        #can_continue = False
        breadth_list = []
        #split_move_list = [[],[],[],[]]

        # Up direction
        if (cur_pos[1] - 1 >= 0): # Checking if inside 2d array
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(0)
                    breadth_list.append([[cur_pos[0], cur_pos[1] - 1], temp_move_list])
                    #split_move_list[0].append(0)
                    #can_continue = True
        
        # Right direction
        if (cur_pos[0] + 1 <= self.map_grid.shape[1] - 1): # Checking if inside 2d array
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(1)
                    breadth_list.append([[cur_pos[0] + 1, cur_pos[1]], temp_move_list])
                    #split_move_list[1].append(1)
                    #can_continue = True

        # Down direction
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1): # Checking 
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0]][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(2)
                    breadth_list.append([[cur_pos[0], cur_pos[1] + 1], temp_move_list])
                    #split_move_list[2].append(2)
                    #can_continue = True

        # Left direction
        if (cur_pos[0] - 1 >= 0): # Checking
            # Check if unvisited by DFS
            if (self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] - 1][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(3)
                    breadth_list.append([[cur_pos[0] - 1, cur_pos[1]], temp_move_list])
                    #split_move_list[3].append(3)
                    #can_continue = True

        #if (can_continue == False):
            #print("false")
        return False, breadth_list
        
        #print(breadth_list)
        #print("")

        #for i in range(len(breadth_list)):
        #    is_frontier_found, temp_move_list = self.move_to_frontier_bfs(breadth_list[i], end_pos, split_move_list[i])
        #    if (is_frontier_found == True):
        #        move_list.extend(temp_move_list)
        #        print("hmm")
        #        return True, move_list
        #    else:
        #        continue

        #print("broken")

    def mtfb_wrapper(self, cur_pos, end_pos, move_list):
        points = [[cur_pos, move_list]]
        # Marking the current point as visited by our BFS. Visited = 3 (just an arbitrary number lol)
        self.map_grid[cur_pos[1]][cur_pos[0]][3] = 3
        while True:
            level_points = []
            for point in points:
                is_found, temp_points = self.move_to_frontier_bfs(point[0], end_pos, point[1])
                level_points.extend(temp_points)

                #print(temp_points)

                if (is_found == True):
                    #print("found")
                    return temp_points[0][1]

            points = level_points

            #print("while loop")


    def draw_frontiers(self, og_map_grid, top_x, top_y):        
        self.map_grid = og_map_grid

        self.local_top_x = top_x
        self.local_top_y = top_y
        self.local_bot_x = top_x + 14
        self.local_bot_y = top_y + 10

        #print(self.local_top_x, self.local_top_y)
        #print(self.local_bot_x, self.local_bot_y)
        
        for i in range(0, len(self.map_grid)):
            for j in range(0, len(self.map_grid[i])):
                if (j > self.local_top_x and j < self.local_bot_x) and \
                    (i > self.local_top_y and i < self.local_bot_y):
                    self.map_grid[i][j][3] = 1
                    if (np.array_equal(self.map_grid[i][j][:3], [0, 0, 0])):
                        self.map_grid[i][j] = [255, 255, 255, 1]
                elif (self.map_grid[i][j][3] == 1):
                    if (np.array_equal(self.map_grid[i][j][:3], [0, 0, 0])):
                        self.map_grid[i][j][:3] = [255, 255, 255]

        return self.map_grid
        
    def get_next_frontier(self, top_x, top_y, og_map_grid):
        self.map_grid = og_map_grid.copy()
        x = 0
        y = 0
        while True:
            x = random.randint(0, self.map_grid.shape[1] - 1)
            y = random.randint(0, self.map_grid.shape[0] - 1)
            if (np.array_equal(self.map_grid[y][x][:3], [0, 0, 0])):
                break
            else:
                continue
        cur_pos = [x, y] # Can randomize this to one of the corners of the map
        print(cur_pos)
        self.frontier_list = []


        print("Finding next frontier to move to...")
        self.ffb_wrapper(cur_pos)
        #self.find_frontier_bfs(cur_pos)
        # This returns the frontier with the smallest (largest) score
        print(self.frontier_list)
        self.next_frontier = [self.frontier_list[0][0], self.frontier_list[0][1], self.frontier_list[0][2]] 
        end_pos = [self.next_frontier[1], self.next_frontier[2]]
        print("Next frontier found at: " + str(self.next_frontier))

        # Get list of moves required to reach frontier
        self.map_grid = og_map_grid.copy()
        print("Getting predicted moves to frontier...")
        move_list = []
        cur_pos = [top_x + 7, top_y + 5]
        move_list = self.mtfb_wrapper(cur_pos, end_pos, move_list)
        #ret_val, move_list = self.move_to_frontier_bfs(cur_pos, end_pos, move_list)
        print(move_list)
        print("Path found. Executing...")

        return move_list

    def frontier_path_collision_handler(self, og_map_grid, top_x, top_y):
        self.map_grid = og_map_grid.copy()
        cur_pos = [top_x + 7, top_y + 5]

        self.consecutive_collisions += 1

        if (self.consecutive_collisions >= 5):
            self.consecutive_collisions = 0
            return False
        else:
            print("Starting course correction...")
            print(self.next_frontier[1:])
            new_moves = []
            new_moves = self.mtfb_wrapper(cur_pos, self.next_frontier[1:], new_moves)
            print(new_moves)
            return new_moves