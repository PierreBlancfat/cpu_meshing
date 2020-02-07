#ifndef POINT_HPP
#define POINT_HPP



struct Point {
	int index;
	float x;
	float y;
	Point(float xIn, float yIn, int ind = -1) : x(xIn), y(yIn), index(ind){};
};

#endif