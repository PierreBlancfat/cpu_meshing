#include <meshing.h>
#include <vector>

void draw_point(sf::RenderWindow &window, int x, int y){
    sf::RectangleShape s{sf::Vector2f(4, 4)};
    s.setPosition(static_cast<float>(x), static_cast<float>(y));
    window.draw(s);
}



void draw_line(sf::RenderWindow &window, int x1, int y1, int x2, int y2){
    sf::Vertex line[2];
    line[0].position = sf::Vector2f(x1, y1);
    line[0].color  = sf::Color::Red;
    line[1].position = sf::Vector2f(x2, y2);
    line[1].color = sf::Color::Red;
    window.draw(line, 2, sf::Lines);
}


void draw_points(sf::RenderWindow &window, MatrixXd points){
    std::cout << points.rows() << std::endl;
    for(int i = 0; i < points.rows(); i++){
        draw_point(window, points(i,0), points(i,1));
    }
}


int main(int argc, char **argv){

    int numberPoints = 40;
	if (argc>1)
	{
		numberPoints = atoi(argv[1]);
	}

	std::default_random_engine eng(std::random_device{}());
	std::uniform_real_distribution<double> dist_w(0, 800);
	std::uniform_real_distribution<double> dist_h(0, 800);

    MatrixXd point = MatrixXd::Random(100,2)*800;
    point = point.array().abs();

    sf::RenderWindow window(sf::VideoMode(800, 600), "Delaunay triangulation");
    window.setFramerateLimit(1);


    draw_point(window, 400,400);
    draw_point(window, 200,400);
    draw_point(window, 200,400);
    draw_line(window, 100,0,300,400);
    draw_points(window, point);


	window.display();


	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
	}

	return 0;
}
