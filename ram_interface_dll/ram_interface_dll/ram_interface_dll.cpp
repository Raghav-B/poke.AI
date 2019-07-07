// ram_interface_dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ram_interface_dll.h"

static unsigned int x_pos;
static unsigned int y_pos;
static unsigned int direction;

static bool allow_data_to_python = false;
static bool allow_data_from_cpp = false;

unsigned int python_get_x_pos() {
    return x_pos;
}
unsigned int python_get_y_pos() {
    return y_pos;
}
unsigned int python_get_direction() {
    return direction;
}

void cpp_set_x_pos(unsigned int input) {
    x_pos = input;
}
void cpp_set_y_pos(unsigned int input) {
    y_pos = input;
}
void cpp_set_direction(unsigned int input) {
    direction = input;
}



