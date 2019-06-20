def get_dist(prev_point, cur_point):
    y_square = (cur_point[1] - prev_point[1]) ** 2
    x_square = (cur_point[0] - prev_point[0]) ** 2
    return (x_square + y_square) ** (1/2)

def sort_cur_midpoints(prev_midpoints, cur_midpoints):
    for cur_point in cur_midpoints:
        smallest_dist = 9999
        smallest_dist_index = None
        
        for prev_point in prev_midpoints:
            if (prev_point[2] == 1):
                continue

            new_test_dist = get_dist(prev_point[0], cur_point[0])
            #print(new_test_dist)
            if (smallest_dist >= new_test_dist):
                smallest_dist = new_test_dist
                smallest_dist_index = prev_point[1]
                prev_point[2] = 1
            else:
                continue

        cur_point[1] = smallest_dist_index

    return prev_midpoints, cur_midpoints



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
