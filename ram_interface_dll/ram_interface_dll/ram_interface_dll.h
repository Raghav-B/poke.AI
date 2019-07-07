#pragma once

#ifdef RAMINTERFACEDLL_EXPORTS
#define RAMINTERFACEDLL_API __declspec(dllexport)
#else
#define RAMINTERFACEDLL_API __declspec(dllexport)
#endif

extern "C" RAMINTERFACEDLL_API unsigned int python_get_x_pos();
extern "C" RAMINTERFACEDLL_API unsigned int python_get_y_pos();
extern "C" RAMINTERFACEDLL_API unsigned int python_get_direction();

extern "C" RAMINTERFACEDLL_API void cpp_set_x_pos(unsigned int input);
extern "C" RAMINTERFACEDLL_API void cpp_set_y_pos(unsigned int input);
extern "C" RAMINTERFACEDLL_API void cpp_set_direction(unsigned int input);