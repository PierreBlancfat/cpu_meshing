#include <meshing.hpp>

using namespace Eigen;
using namespace std;

int main(int argc, char **argv) {

  sf::RenderWindow window(sf::VideoMode(800, 800), "Delaunay triangulation");
  Meshing mesh = Meshing(800, 800, &window);
  mesh.draw_points();
  int fin = mesh.triangulation_rec(1);
}
