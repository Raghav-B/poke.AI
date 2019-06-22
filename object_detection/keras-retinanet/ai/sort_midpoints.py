class midpoint_sorter:
    min_thresh_dist = 40
    max_thresh_dist = 100
    cur_indexes = set()
    init_index = -1

    def get_init_index(self):
        self.init_index += 1
        return self.init_index

    # A bit slow, because it runs linearly every frame, not so much an issue when there are only
    # very few objects to be detected on screen.
    def find_next_free_index(self):
        start = 0
        while True:
            if (start in self.cur_indexes) == True:
                start += 1
            else:
                #self.cur_indexes.add(start)
                return start

    def get_dist(self, prev_point, cur_point):
        y_square = (cur_point[1] - prev_point[1]) ** 2
        x_square = (cur_point[0] - prev_point[0]) ** 2
        return ((x_square + y_square) ** 0.5)

    def sort_cur_midpoints(self, prev_midpoints, cur_midpoints):
        i = 0
        self.cur_indexes = set()
        
        while (i < len(cur_midpoints)):
            j = 0
            while (j < len(prev_midpoints)):
                temp_dist = self.get_dist(prev_midpoints[j][0], cur_midpoints[i][0])
                #print(temp_dist)
                if (temp_dist <= self.min_thresh_dist):
                    cur_midpoints[i][1] = prev_midpoints[j][1]
                    self.cur_indexes.add(prev_midpoints[j][1])
                    #print("broken")
                    break
                    # We can safely break because we assume that there is only one possible point that
                    # in such close proximity to our current point to be indexed.
                # In this event find the point closest to this one
                j += 1
            i += 1

        for new_point in cur_midpoints:
            if new_point[1] == -1:
                new_point[1] = self.find_next_free_index()
                self.cur_indexes.add(new_point[1])

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
