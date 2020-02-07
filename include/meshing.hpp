#ifndef MESHING_HPP
#define MESHING_HPP


#include "hull.hpp"
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <SFML/Graphics.hpp>


class Meshing{

    private:

        sf::RenderWindow *window;
        std::vector<Point> points;    

    public: 

        Meshing(int witdh, int height, sf::RenderWindow  *win );
        
        void draw_point(int x, int y);

        void draw_points();

        void draw_line(int x1, int y1, int x2, int y2);

        int convex_hull(Eigen::MatrixXd points);

        int triangulation();


};

int points_to_matrix(std::vector<Point> vect_points);
int main(int argc, char **argv);


#endif