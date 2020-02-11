#ifndef MESHING_HPP
#define MESHING_HPP


#include <iostream>
#include <list>
#include <vector>
#include <Eigen/Dense>
#include <SFML/Graphics.hpp>

#include "hull.hpp"

class Meshing{

    private:

        sf::RenderWindow *window;
        std::vector<Point> points;    

    public: 

        Meshing(int witdh, int height, sf::RenderWindow  *win );
        
        void draw_point(int x, int y, sf::Color= sf::Color::White);
        void draw_points();
        void draw_points(std::vector<Point> points, sf::Color color = sf::Color::White);
        void draw_line(float x1, float y1, float x2, float y2,  sf::Color color = sf::Color::White);
        void draw_triangle( Triangle t, sf::Color color = sf::Color::White);

        int ParDeTri(std::vector<Point> points_set, std::vector<Edge> edge_list);
        int convex_hull(Eigen::MatrixXd points);
        int side(Point p, std::vector<Point> &path);
        int partition_1();
        std::vector<Point> partition_path();


};

float dd(Edge e, Point p);
std::vector<Edge> point_vect_to_vect_edge(std::vector<Point> &ps);
int nearest_point(std::vector<Point> &ps, Edge &e);
void update(Edge &e, std::vector<Edge> &L);
int points_to_matrix(std::vector<Point> vect_points);
int main(int argc, char **argv);


#endif