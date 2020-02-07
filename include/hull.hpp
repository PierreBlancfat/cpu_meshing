#include "point.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>


// Implementations of various convex hull algorithms 
// using the C++ Standard Library.
// For clarity, the implementations do not check for
// duplicate or collinear points.

// The z-value of the cross product of segments 
// (a, b) and (a, c). Positive means c is ccw
// from (a, b), negative cw. Zero means its collinear.
float ccw(const Point& a, const Point& b, const Point& c);
// Returns true if a is lexicographically before b.
bool isLeftOf(const Point& a, const Point& b);
// Used to sort Points in ccw order about a pivot.
struct ccwSorter;

// The length of segment (a, b).
float len(const Point& a, const Point& b);

// The unsigned distance of p from segment (a, b).
float dist(const Point& a, const Point& b, const Point& p);
// Returns the index of the farthest Point from segment (a, b).
size_t getFarthest(const Point& a, const Point& b, const std::vector<Point>& v);

// The gift-wrapping algorithm for convex hull.
// https://en.wikipedia.org/wiki/Gift_wrapping_algorithm
std::vector<Point> giftWrapping(std::vector<Point> v);
// The Graham scan algorithm for convex hull.
// https://en.wikipedia.org/wiki/Graham_scan
std::vector<Point> GrahamScan(std::vector<Point> v);

// The monotone chain algorithm for convex hull.
std::vector<Point> monotoneChain(std::vector<Point> v);

// Recursive call of the quickhull algorithm.
void quickHull(const std::vector<Point>& v, const Point& a, const Point& b, std::vector<Point>& hull) ;
// QuickHull algorithm. 
// https://en.wikipedia.org/wiki/QuickHull
std::vector<Point> quickHull(const std::vector<Point>& v);

std::vector<Point> getPoints() ;
void print(const std::vector<Point>& v);

