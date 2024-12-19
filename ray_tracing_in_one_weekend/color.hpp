#pragma once
#ifndef COLOR_H
#define COLOR_H
#include "interval.hpp"
#include "vec3.hpp"

using color = vec3;

inline double linear_to_gamma(double linear_component)
{
	//gamma 2 correction
	if (linear_component > 0)
		return std::sqrt(linear_component);
	return 0;
}

void write_color(std::ostream& out, const color& pixel_color);


#endif