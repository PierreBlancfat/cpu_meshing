#include <meshing.h>
#include <vector>

int main(int argc, char **argv){

    int numberPoints = 40;
	if (argc>1)
	{
		numberPoints = atoi(argv[1]);
	}

	std::default_random_engine eng(std::random_device{}());
	std::uniform_real_distribution<double> dist_w(0, 800);
	std::uniform_real_distribution<double> dist_h(0, 800);

    MatrixXd point = MatrixXd::Random(2,1000)*800;
    point = point.array().abs();

    sf::RenderWindow window(sf::VideoMode(800, 600), "Delaunay triangulation");
    window.setFramerateLimit(1);

    // Transform each points of each vector as a rectangle
    for(int i = 0; i < 100; i++){
        sf::RectangleShape s{sf::Vector2f(4, 4)};
        s.setPosition(static_cast<float>(point(0,i)), static_cast<float>(point(1,i)));
        window.draw(s);
    }

    // std::vector<std::array<sf::Vertex, 2> > lines;
	// for(const auto &e : edges) {
	// 	const std::array<sf::Vertex, 2> line{{
	// 		sf::Vertex(sf::Vector2f(
	// 				static_cast<float>(e.v->x + 2.),
	// 				static_cast<float>(e.v->y + 2.))),
	// 		sf::Vertex(sf::Vector2f(
	// 				static_cast<float>(e.w->x + 2.),
	// 				static_cast<float>(e.w->y + 2.))),
	// 	}};
	// 	window.draw(std::data(line), 2, sf::Lines);
	// }

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
