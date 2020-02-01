#include <meshing.h>


int main(int argc, char **argv)
{
  MatrixXf mesh = MatrixXf::Random(2,100)*10;
  std::cout << mesh << std::endl;

}
