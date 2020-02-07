#include "meshing.hpp"
#include "hull.cpp"

using namespace std;
using namespace Eigen;


Meshing::Meshing(int witdh, int height, sf::RenderWindow  *win ){
    window = win;
    window->setFramerateLimit(1);
    MatrixXd points_eigen = MatrixXd::Random(100,2)*witdh;
    points_eigen = points_eigen.array().abs();

    for(int i = 0; i < points_eigen.rows(); i++){
        points.push_back(Point(points_eigen(i,0), points_eigen(i,1)));
    }
}


int Meshing::triangulation(){
    vector<Point> proj(points.size());
    proj = copy(points.begin(), points.end(), proj.begin());
    // median
    float median_x =  x_coord.mean(); // TODO really take median !!!
    //3d projection
    MatrixXd proj(points.rows(), 2);
    for(int i = 0; i < points.rows(); i++){
        proj(i,0) = points(i,0);
        proj(i,1) = pow(points(i,0)-median_x, 2) + pow(points(i,1),2);
    }
    std::cout << proj << std::endl;
    return points;
}


void Meshing::draw_point(int x, int y){
    sf::RectangleShape s{sf::Vector2f(4, 4)};
    s.setPosition(static_cast<float>(x), static_cast<float>(y));
    window->draw(s);
}


void Meshing::draw_points(){
    for(int i = 0; i < points.size(); i++){
        draw_point(points[i].x, points[i].y);
    }
}

void Meshing::draw_line(int x1, int y1, int x2, int y2){
    sf::Vertex line[2];
    line[0].position = sf::Vector2f(x1, y1);
    line[0].color  = sf::Color::Red;
    line[1].position = sf::Vector2f(x2, y2);
    line[1].color = sf::Color::Red;
    window->draw(line, 2, sf::Lines);

}


int points_to_matrix(vector<Point> vect_points){
    for( int i = 0; i < vect_points.size(); i++){
        std:cout << i << std::endl;
    }
    return 1;
}



int main(int argc, char **argv){


    sf::RenderWindow window(sf::VideoMode(800, 600), "Delaunay triangulation");
    Meshing mesh = Meshing(800, 800, &window);
    mesh.draw_points();

    // triangulation(points);
    convex_hull(points);

	window.display();

	// while (window.isOpen())
	// {
	// 	sf::Event event;
	// 	while (window.pollEvent(event))
	// 	{
	// 		if (event.type == sf::Event::Closed)
	// 			window.close();
	// 	}
	// }

	return 0;
}
 