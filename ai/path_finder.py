"""
We can use a frontier based exploration algorithm, where frontiers are determined in terms of their open space.
Frontier points will be any points that are on the edge of our current grid_map. Points that have come inside the frame are considered detected, while those not on our frame are considered undetected.

To find frontier points, we basically do a breadth first search of all unvisited edges of the map, and calculate a score for each potential frontier point. We can store these inside a priority queue so that we only retrieve the point that is bound to give us the highest potential score.

This potential score measure is based on a measure of proximity to buildings, open space (potential for exploration) and. 

Start of BFS for frontier detection:
    - We can start the BFS from the top left corner of the frame.

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
    def __init__(self):
        self.local_top_x = 0
        self.local_top_y = 0
        self.local_bot_x = 0
        self.local_bot_y = 0

        self.map_grid = None
        self.frontier_list = []
        self.unreachable_frontiers = set()
        self.next_frontier = None

        self.consecutive_movements = 0
        self.consecutive_collisions = 0
        self.consecutive_collisions_limit = 5

    def get_frontier_score(self, query_pos):
        if (np.array_equal(query_pos[:3], [0, 0, 0])): # Unvisited
            return 20
        elif (np.array_equal(query_pos[:3], [255, 255, 255])): # Visited
            return 0
        elif (np.array_equal(query_pos[:3], [96, 102, 30])): # Gym
            return 100
        elif (np.array_equal(query_pos[:3], [30, 57, 102])): # House
            return 70
        elif (np.array_equal(query_pos[:3], [0, 0, 255])): # Pokecen
            return 55
        elif (np.array_equal(query_pos[:3], [255, 0, 0])): # Pokemart
            return 40
        elif (np.array_equal(query_pos[:3], [105, 105, 105])): # Wall/Boundary
            return -40
        elif (np.array_equal(query_pos[:3], [66, 135, 245])): # NPC
            return 65

    def find_frontier_bfs(self, cur_pos):
        frontier_breadth_list = []
        score = 0

        # Up direction
        if (cur_pos[1] - 1 >= 0): # Checking if not out of bounds of 2d map_grid array
            score += self.get_frontier_score(self.map_grid[cur_pos[1] - 1][cur_pos[0]])
            # Check if unvisited by BFS
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] != 3):
                # Point needs to be a black, unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] = 3
                    frontier_breadth_list.append([cur_pos[0], cur_pos[1] - 1])
        
        # Right direction
        if (cur_pos[0] + 1 <= self.map_grid.shape[1] - 1):
            score += self.get_frontier_score(self.map_grid[cur_pos[1]][cur_pos[0] + 1])
            if (self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] + 1, cur_pos[1]])

        # Down direction
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] + 1][cur_pos[0]])
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] = 3
                    frontier_breadth_list.append([cur_pos[0], cur_pos[1] + 1])

        # Left direction
        if (cur_pos[0] - 1 >= 0):
            score += self.get_frontier_score(self.map_grid[cur_pos[1]][cur_pos[0] - 1])
            if (self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] - 1, cur_pos[1]])

        # Top left
        if (cur_pos[1] - 1 >= 0 and cur_pos[0] - 1 >= 0):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] - 1][cur_pos[0] - 1])
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0] - 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] = 3
                    frontier_breadth_list.append([cur_pos[0] - 1, cur_pos[1] - 1])

        # Top right
        if (cur_pos[1] - 1 >= 0 and cur_pos[0] + 1 <= self.map_grid.shape[1] - 1):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1])
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0] + 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] + 1, cur_pos[1] - 1])

        # Bottom left
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1 and cur_pos[0] - 1 >= 0):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1])
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0] - 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] - 1, cur_pos[1] + 1])

        # Bottom right
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1 and cur_pos[0] + 1 <= self.map_grid.shape[1] - 1):
            score += self.get_frontier_score(self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1])
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0] + 1][3] = 3
                    frontier_breadth_list.append([cur_pos[0] + 1, cur_pos[1] + 1])

        # Pushing current frontier to priority queue, only if frontier is reachable
        if (not ((cur_pos[0], cur_pos[1]) in self.unreachable_frontiers)):
            pq.heappush(self.frontier_list, [-score, cur_pos[0], cur_pos[1]])
        
        return frontier_breadth_list # Returns list of points we've reached at our current search depth
    
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

        breadth_list = []
        #can_continue = False

        # Up direction
        if (cur_pos[1] - 1 >= 0): # Checking if not out of bounds of 2d array
            # Check if unvisited by BFS
            if (self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] != 3):
                # Point needs to be a black or white unvisited point. Otherwise we will collide into a building
                if (np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1] - 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] - 1][cur_pos[0]][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(0)
                    breadth_list.append([[cur_pos[0], cur_pos[1] - 1], temp_move_list])
                    #can_continue = True

        # Right direction
        if (cur_pos[0] + 1 <= self.map_grid.shape[1] - 1):
            if (self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] + 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] + 1][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(1)
                    breadth_list.append([[cur_pos[0] + 1, cur_pos[1]], temp_move_list])
                    #can_continue = True

        # Down direction
        if (cur_pos[1] + 1 <= self.map_grid.shape[0] - 1):
            if (self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0]][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1] + 1][cur_pos[0]][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1] + 1][cur_pos[0]][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(2)
                    breadth_list.append([[cur_pos[0], cur_pos[1] + 1], temp_move_list])
                    #can_continue = True

        # Left direction
        if (cur_pos[0] - 1 >= 0):
            if (self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] != 3):
                if (np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] - 1][:3], [255, 255, 255]) or \
                    np.array_equal(self.map_grid[cur_pos[1]][cur_pos[0] - 1][:3], [0, 0, 0])):
                    self.map_grid[cur_pos[1]][cur_pos[0] - 1][3] = 3
                    temp_move_list = move_list.copy()
                    temp_move_list.append(3)
                    breadth_list.append([[cur_pos[0] - 1, cur_pos[1]], temp_move_list])
                    #can_continue = True

        # Returns whether our frontier point has been found, along with the list of points at our current BFS search level
        return False, breadth_list#, can_continue

    def mtfb_wrapper(self, cur_pos, end_pos, move_list):
        points = [[cur_pos, move_list]]
        # Marking the current point as visited by our BFS. Visited = 3 (just an arbitrary number lol)
        self.map_grid[cur_pos[1]][cur_pos[0]][3] = 3
        while True:
            level_points = []
            #can_continue = False
            for point in points:
                is_found, temp_points = self.move_to_frontier_bfs(point[0], end_pos, point[1])
                level_points.extend(temp_points)

                #if (can_continue == False):
                #    return False

                if (is_found == True):
                    return temp_points[0][1]

            points = level_points
        
    def get_next_frontier(self, top_x, top_y, og_map_grid):
        self.map_grid = og_map_grid.copy()
        
        # Getting a random unvisited black point to start our search for best frontier from.
        x = 0
        y = 0
        while True:
            x = random.randint(0, self.map_grid.shape[1] - 1)
            y = random.randint(0, self.map_grid.shape[0] - 1)
            if (np.array_equal(self.map_grid[y][x][:3], [0, 0, 0])):
                break
            else:
                continue
        cur_pos = [x, y]
        print("Frontier BFS start point: " + str(cur_pos))

        print("Finding next frontier to move to...")
        # Resetting frontier pq to ensure we do not double count with our previous frontiers.
        self.frontier_list = []
        # Getting pq of best frontiers to go to
        self.ffb_wrapper(cur_pos)
        
        # Getting frontier with highest score
        # We introduce a bit of randomness here because otherwise the pq makes the root the top left corner point always.
        frontier_index = 0
        if (len(self.frontier_list) < 8):
            frontier_index = random.randint(0, len(self.frontier_list) - 1)
        else:
            frontier_index = random.randint(0, 7)
        self.next_frontier = self.frontier_list[frontier_index] 
        print("Next frontier found at: " + str(self.next_frontier))

        # Get list of moves required to reach our selected frontier
        self.map_grid = og_map_grid.copy()
        print("Getting predicted moves to frontier...")
        move_list = []
        cur_pos = [top_x + 7, top_y + 5] # Resetting cur_pos variable to our agent's current global position
        # Getting move_list of moves required to get to our chosen frontier
        move_list = self.mtfb_wrapper(cur_pos, self.next_frontier[1:], move_list)
        #if (move_list == False):
        #    continue
        print(move_list)
        print("Path found. Executing...")

        return move_list

    def frontier_path_collision_handler(self, og_map_grid, top_x, top_y):
        self.map_grid = og_map_grid.copy()
        cur_pos = [top_x + 7, top_y + 5]

        self.consecutive_collisions += 1
        # If a collision has occured our consecutive movements are naturally reset back to 0
        self.consecutive_movements = 0
        print("Consecutive collisions: " + str(self.consecutive_collisions))

        if (self.consecutive_collisions >= self.consecutive_collisions_limit):
            self.consecutive_collisions = 0
            print("Too many consecutive collisions!")
            print("Switching focus to new frontier...")
            self.unreachable_frontiers.add((self.next_frontier[1], self.next_frontier[2]))
            return False
        else:
            print("Starting course correction...")
            print("Continuing movement to frontier: " + str(self.next_frontier))
            # Getting new list of moves after considering any new developments in the map
            print("Getting correct moves to frontier...")
            new_moves = []
            new_moves = self.mtfb_wrapper(cur_pos, self.next_frontier[1:], new_moves)
            print(new_moves)
            
            return new_moves