#include <meshing.hpp>

using  namespace Eigen;
using  namespace std;

int main(int argc, char **argv){


  sf::RenderWindow window(sf::VideoMode(800, 800), "Delaunay triangulation");
  Meshing mesh = Meshing(800, 800, &window);
  mesh.draw_points();
  window.display();

	// return 1;
 int fin = mesh.triangulation_rec(1);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
	}
	return 1;
}
