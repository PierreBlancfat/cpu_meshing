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
        void draw_line(float x1, float y1, float x2, float y2);

        int convex_hull(Eigen::MatrixXd points);

        int side(Point p, std::vector<Point> &path);
        int* partition_1();
        std::vector<Point> partition_path();


};

int points_to_matrix(std::vector<Point> vect_points);
int main(int argc, char **argv);


#endif