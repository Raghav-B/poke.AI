class midpoint_sorter:
    self.min_thresh_dist = 5 # 10
    self.cur_indexes = {}
    self.init_index = -1

    #def __init__(self, prev_midpoints):
    #    index = 0
    #    for point in prev_midpoints:

    def get_init_index(self):
        self.init_index += 1
        self.prev_indexes.add(self.init_index)
        return self.init_index

    # A bit slow, because it runs linearly every frame, not so much an issue when there are only
    # very few objects to be detected on screen.
    def find_next_free_index(self):
        start = 0
        while True:
            if (start in self.cur_indexes) == True:
                start += 1
            else:
                return start

    def get_dist(prev_point, cur_point):
        y_square = (cur_point[1] - prev_point[1]) ** 2
        x_square = (cur_point[0] - prev_point[0]) ** 2
        return (x_square + y_square) ** (1/2)

    def sort_cur_midpoints(self, prev_midpoints, cur_midpoints):
        i = 0
        self.cur_indexes = {}
        
        while (i < len(cur_midpoints)):
            j = 0
            while (j < len(prev_midpoints)):
                temp_dist = get_dist(prev_midpoints[i][0], cur_midpoints[j][0])
                if (temp_dist <= min_thresh_dist):
                    cur_midpoints[i][1] = prev_midpoints[j][1]
                    self.cur_indexes.add(prev_midpoints[j][1])
                    break
                    # We can safely break because we assume that there is only one possible point that
                    # in such close proximity to our current point to be indexed.
                else:
                    continue

            # This is when a new midpoint has been detected since the previous frame
            if (cur_midpoints[i][1] == -1):
                cur_midpoints[i][1] = find_next_free_index()
                self.cur_indexes.add(cur_midpoints[i][1])

        # If a midpoint has been lost in the new frame as compared to the prev frame.
        #for i in self.prev_indexes: 
            # If old object is not found in new frame.
            #if (i in self.cur_indexes) == False:
        return cur_midpoints



"""
# Handling new midpoints
Assign an index -1 to all detected midpoints.
If a new midpoint is found in the detected midpoints, 

Since the current frame can have new midpoints, it is better for the prev points to be mapped to the cur points.
If an index for a certain point remains -1, we know it is a new point, and so we can assign it a new unused index.

Have a set of used indexes to denote which indexes we cant use. We iterate through 0 to N and check if any of these indexes
exist in the set. If they do, we don't 

# Handlng disappeared midpoints

"""
