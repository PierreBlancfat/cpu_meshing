#define __CL_ENABLE_EXCEPTIONS
#define MAXFLOAT 6666666666

#include "meshing.hpp"
#include "circumcenter.cpp"
#include "cl.hpp"
#include "device_picker.hpp"
#include "hull.cpp"
#include "util.hpp" // utility library

using namespace std;
using namespace Eigen;

// pick up device type from compiler command line or from the default type
#ifndef DEVICE
#define DEVICE CL_DEVICE_TYPE_CPU
#endif

#define TOL (0.001)      // tolerance used in floating point comparisons
#define LENGTH (1 << 23) // length of vectors a, b, and c

// TRIANGULATION


Meshing::Meshing(int witdh, int height, sf::RenderWindow *win) {
  window = win;
  window->setFramerateLimit(1);
  MatrixXd points_eigen = MatrixXd::Random(1000000, 2) * witdh;
  points_eigen = points_eigen.array().abs();

  for (int i = 0; i < points_eigen.rows(); i++) {
    points.push_back(Point(points_eigen(i, 0), points_eigen(i, 1), i));
  }
}

int Meshing::triangulation(int nb_partition) {
  long delta = 0;

  vector<Partition> partitions;
  vector<Edge> edge_path;
  vector<Triangle> triangle_list;

  for (int i = 0; i < nb_partition; i++) {
    vector<Point> H1, H2, path;
    path = partition(points, H1, H2, true);
    edge_path = point_vect_to_vect_edge(path);
    Partition P1 = Partition(H1, edge_path);
    Partition P2 = Partition(H2, edge_path);
    partitions.push_back(P1);
  }

  return 1;
  for (int i = 0; i < partitions.size(); i++) {
    ParDeTri(partitions[i].partition, partitions[i].path, triangle_list);
  }

  for (int i = 0; i < triangle_list.size(); i++) {
    draw_triangle(triangle_list[i]);
  }
}


int Meshing::triangulation_rec(int nb_partition) {
  vector<Partition> partitions;
  vector<Point> path;
  vector<Triangle> triangle_list;
  partitionRec(points, path, true, nb_partition, partitions);

  // #pragma omp parallel for
  for (int i = 0; i < partitions.size(); i++) {
    ParDeTri(partitions[i].partition, partitions[i].path, triangle_list);
  }

  for (int i = 0; i < triangle_list.size(); i++) {
    draw_triangle(triangle_list[i]);
  }
}

//recursively make partitions
void Meshing::partitionRec(vector<Point> points_set, vector<Point> old_path,
                           bool vertical, int deph_rec,
                           vector<Partition> &partitions) {
  if (deph_rec > 1) {
    vector<Point> H1, H2, path;
    path = partition(points_set, H1, H2, vertical, old_path);
    partitionRec(H1, path, !vertical, deph_rec - 1, partitions);
    partitionRec(H2, path, !vertical, deph_rec - 1, partitions);
  } else {
    vector<Point> H1, H2, path;
    vector<Edge> edge_path;
    path = partition(points_set, H1, H2, vertical, old_path);
    edge_path = point_vect_to_vect_edge(path);
    partitions.push_back(Partition(H1, edge_path));
    partitions.push_back(Partition(H2, edge_path));
  }
}

void Meshing::ParDeTri(vector<Point> point_set, vector<Edge> edge_list,
                       vector<Triangle> &triangle_list) {
  int index_nearest_point;
  int it = 0;
  int max_it = 50;
  while (edge_list.size() > 0 && it < max_it) {
    it++;
    // pop first edge
    int i = 0;
    Edge e = Edge(edge_list[i].one, edge_list[i].two);
    edge_list.erase(edge_list.begin() + i);

    // make a delaunay triangle
    index_nearest_point = nearest_point_gpu(point_set, e);
    Triangle t = Triangle(e, point_set[index_nearest_point]);

    Edge ep = Edge(e.one, point_set[index_nearest_point]);
    Edge ep2 = Edge(e.two, point_set[index_nearest_point]);
    // point_set.erase(point_set.begin()+index_nearest_point);
    // Update
    if (t.is_triangle()) {
      triangle_list.push_back(t);
      update(ep, edge_list);
      update(ep2, edge_list);
    } else
      cout << "not triangle" << endl;
  }
  cout << "Finished ParDeTri" << endl;
  return;
}

void update(Edge e, vector<Edge> &edge_list) {
  bool find = false;
  for (int i = 0; i < edge_list.size(); i++) {
    if (e == edge_list[i]) {
      edge_list.erase(edge_list.begin() + i);
      return;
    }
  }
  edge_list.push_back(e);
}

std::vector<Point> Meshing::partition(std::vector<Point> list_points,
                                      vector<Point> &H1, vector<Point> &H2,
                                      bool vertical, vector<Point> old_path) {
  vector<Point> path;
  path = partition_path(list_points, vertical, old_path);
  int s;
  cout << " ********** partition ******" << endl;
  // fill partitions
  for (int i = 0; i < list_points.size(); i++) {
    if (!std::count(path.begin(), path.end(), list_points[i])) {
      s = side(list_points[i], path, vertical);
      if (s < 0) {
        draw_point(list_points[i].x, list_points[i].y, sf::Color::Green);
        H1.push_back(list_points[i]);
      }
      if (s >= 0) {
        draw_point(list_points[i].x, list_points[i].y, sf::Color::Blue);
        H2.push_back(list_points[i]);
      }
    }
  }
  for (int i = 0; i < path.size(); i++) {
    H1.push_back(path[i]);
    H2.push_back(path[i]);
    draw_point(path[i].x, path[i].y, sf::Color::Yellow);
  }
  return path;
}


vector<Point> Meshing::partition_path(std::vector<Point> list_points,
                                      bool vertical, vector<Point> old_path) {
  // copy vector
  vector<Point> proj;

  // find median
  std::vector<float> xcoord;
  for (int i = 0; i < list_points.size(); i++) {
    if (vertical) {
      xcoord.push_back(list_points[i].x);
    } else {
      xcoord.push_back(list_points[i].y);
    }
  }
  int n = sizeof(xcoord) / sizeof(xcoord[0]);
  sort(xcoord.begin(), xcoord.end());
  float median = xcoord[xcoord.size() / 2];

  // 3d projection
  vector<Point> hull;
  for (int i = 0; i < list_points.size(); i++) {
    if (vertical) {

      proj.push_back(
          Point(list_points[i].y,
                pow(list_points[i].x - median, 2) + pow(list_points[i].y, 2),
                list_points[i].index));
    } else {
      proj.push_back(
          Point(list_points[i].x,
                pow(list_points[i].y - median, 2) + pow(list_points[i].x, 2),
                list_points[i].index));
    }
  }
  // delauney path :
  hull = quickHull(proj);

  float tolerance = 50;

  // fill path, keep lower convex hull and fraw hull
  vector<Point> path;
  int index, index2;
  bool not_in_path = true;
  for (int i = 0; i < hull.size() - 1; i++) {
    index = hull[i].index;
    index2 = hull[i + 1].index;
    not_in_path = true;
    if (!old_path.empty()) {
      not_in_path = not_in_vect(old_path, points[index]);
    }

    if (vertical == true) {
      if (points[index].y > points[index2].y) {
        if (!not_in_path && (points[index].x - tolerance < median &&
                             points[index].x + tolerance > median)) {
          draw_line(points[index].x, points[index].y, points[index2].x,
                    points[index2].y, sf::Color::Red);
          path.push_back(points[index]);
        } else if (not_in_path) {
          draw_line(points[index].x, points[index].y, points[index2].x,
                    points[index2].y, sf::Color::Red);
          path.push_back(points[index]);
        }
      }
    } else {
      if (points[index].x > points[index2].x) {
        if (!not_in_path && (points[index].y - tolerance < median &&
                             points[index].y + tolerance > median)) {
          draw_line(points[index].x, points[index].y, points[index2].x,
                    points[index2].y, sf::Color::Yellow);
          path.push_back(points[index]);
        } else if (not_in_path) {
          draw_line(points[index].x, points[index].y, points[index2].x,
                    points[index2].y, sf::Color::Yellow);
          path.push_back(points[index]);
        }
      }
    }
  }

  index = hull[hull.size() - 1].index;
  index2 = hull[0].index;

  if (!old_path.empty()) {
    not_in_path = not_in_vect(old_path, points[index]);
  }

  if (vertical == true) {
    if (points[index].y > points[index2].y) {
      if (!not_in_path && (points[index].x - tolerance < median &&
                           points[index].x + tolerance > median)) {
        draw_line(points[index].x, points[index].y, points[index2].x,
                  points[index2].y, sf::Color::Red);
        path.push_back(points[index]);
      } else if (not_in_path) {

        draw_line(points[index].x, points[index].y, points[index2].x,
                  points[index2].y, sf::Color::Red);
        path.push_back(points[index]);
      }
    }
  } else {
    if (points[index].x > points[index2].x) {
      if (!not_in_path && (points[index].y - tolerance < median &&
                           points[index].y + tolerance > median)) {
        draw_line(points[index].x, points[index].y, points[index2].x,
                  points[index2].y, sf::Color::Yellow);
        path.push_back(points[index]);
      } else if (not_in_path) {
        draw_line(points[index].x, points[index].y, points[index2].x,
                  points[index2].y, sf::Color::Yellow);
        path.push_back(points[index]);
      }
    }
  }

  return path;
}

// side of a point p regarding a path
int Meshing::side(Point p, vector<Point> &path, bool vertical) {
  // find segment
  if (vertical == true) {
    for (int i = 1; i < path.size(); i++) {

      if (p.y <= path[i - 1].y && p.y > path[i].y) {
        return (p.x - path[i - 1].x) * (path[i].y - path[i - 1].y) -
               (p.y - path[i - 1].y) * (path[i].x - path[i - 1].x);
      }
    }
    if (p.y <= path[path.size() - 1].y && p.y > path[0].y &&
        path[path.size() - 1].y > path[0].y) {
      return (p.x - path[path.size() - 1].x) *
                 (path[0].y - path[path.size() - 1].y) -
             (p.y - path[path.size() - 1].y) *
                 (path[0].x - path[path.size() - 1].x);
    }
  } else {
    for (int i = 1; i < path.size(); i++) {
      if (p.x <= path[i - 1].x && p.x > path[i].x) {
        return (p.x - path[i - 1].x) * (path[i].y - path[i - 1].y) -
               (p.y - path[i - 1].y) * (path[i].x - path[i - 1].x);
      }
    }
    if (p.y <= path[path.size() - 1].y && p.y > path[0].y &&
        path[path.size() - 1].y > path[0].y) {
      return (p.x - path[path.size() - 1].x) *
                 (path[0].y - path[path.size() - 1].y) -
             (p.y - path[path.size() - 1].y) *
                 (path[0].x - path[path.size() - 1].x);
    }
  }
  return -1;
}

int Meshing::nearest_point_cpu(vector<Point> ps, Edge &e) {

  int len = ps.size();
  float dists[len];

#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < ps.size(); i++) {
    dists[i] = dd(e, ps[i]);
  }

  float min = MAXFLOAT;
  int index_min = -1;
#pragma omp parallel
  {
    int loc_index = index_min;
    float loc_min = min;
#pragma omp parallel for reduction(min : loc_min)
    for (int i = 0; i < len; i++) {
      if (loc_min > dists[i]) {
        loc_min = dists[i];
        loc_index = i;
      }
    }
#pragma omp critical
    {
      if (min > loc_min) {
        min = loc_min;
        index_min = loc_index;
      }
    }
  }
  return index_min;
}

#define LENGTH (1024)
int Meshing::nearest_point_gpu(vector<Point> &ps, Edge &e) {

  // const unsigned int count = ps.size();
  int count = ps.size();
  vector<float> px, py, dists(count, 0xdeadbeef);
  // fill buffer

  for (int i = 0; i < count; i++) {
    px.push_back(ps[i].x);
    py.push_back(ps[i].y);
  }
  // cout << (ps[0].x) << " " << px[0]<< endl;

  cl_uint deviceIndex = 0;
  // parseArguments(argc, argv, &deviceIndex);

  // Get list of devices
  std::vector<cl::Device> devices;
  unsigned numDevices = getDeviceList(devices);

  // Check device index in range
  if (deviceIndex >= numDevices) {
    std::cout << "Invalid device index (try '--list')\n";
    return EXIT_FAILURE;
  }

  cl::Device device = devices[deviceIndex];

  std::string name;
  getDeviceName(device, name);

  std::vector<cl::Device> chosen_device;
  chosen_device.push_back(device);
  cl::Context context(chosen_device);

  cl::Program program(context, util::loadProgram("dists_kernel.cl"), true);

  cl::CommandQueue queue(context);


  auto dists_kernel =
      cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer,
                      cl::Buffer, cl::Buffer, cl::Buffer, int>(program,
                                                               "dists_kernel");

  cl::Buffer d_px;
  cl::Buffer d_py;
  cl::Buffer d_dists;

  cl::Buffer d_onex(context, CL_MEM_READ_WRITE, sizeof(float));
  queue.enqueueWriteBuffer(d_onex, CL_TRUE, 0, sizeof(int), &e.one.x);

  cl::Buffer d_oney(context, CL_MEM_READ_WRITE, sizeof(float));
  queue.enqueueWriteBuffer(d_oney, CL_TRUE, 0, sizeof(int), &e.one.y);

  cl::Buffer d_twox(context, CL_MEM_READ_WRITE, sizeof(float));
  queue.enqueueWriteBuffer(d_twox, CL_TRUE, 0, sizeof(int), &e.two.x);

  cl::Buffer d_twoy(context, CL_MEM_READ_WRITE, sizeof(float));
  queue.enqueueWriteBuffer(d_twoy, CL_TRUE, 0, sizeof(int), &e.two.y);

  d_px = cl::Buffer(context, px.begin(), px.end(), true);
  d_py = cl::Buffer(context, py.begin(), py.end(), true);
  d_dists = cl::Buffer(context, dists.begin(), dists.end(), true);

  dists_kernel(cl::EnqueueArgs(queue, cl::NDRange(count)), d_px, d_py, d_dists,
               d_onex, d_oney, d_twox, d_twoy, count);

  queue.finish();

  cl::copy(queue, d_dists, dists.begin(), dists.end());
  float min = 3000000;
  float dis, min_dist;
  int index_min = -1;
  for (int i = 0; i < ps.size(); i++) {
    if (min > dists[i]) {
      min = dists[i];
      index_min = i;
      min_dist = dists[i];
    }
  }
  return index_min;
}

int Meshing::nearest_point(vector<Point> &ps, Edge &e) {
  float min = MAXFLOAT;
  float dis;
  int index_min = -1;
  for (int i = 0; i < ps.size(); i++) {
    dis = dd(e, ps[i]);
    if (min > dis) {
      min = dis;
      index_min = i;
    }
  }
  return index_min;
}

bool Meshing::is_goodtriangle(vector<Point> &LIST, Point &ps, Edge &e) {
  MatrixXf A(4, 4);
  float det;
  for (int i = 0; i < LIST.size(); i++) {
    if (((e.one.x * e.two.y - e.two.x * e.one.y) +
         (e.two.x * ps.y - ps.x * e.two.y) +
         (ps.x * e.one.y - e.one.x * ps.y)) < 0) {
      A(0, 0) = e.one.x;
      A(0, 1) = e.one.y;
      A(0, 2) = pow(e.one.x, 2) + pow(e.one.y, 2);
      A(0, 3) = 1;
      A(1, 0) = ps.x;
      A(1, 1) = ps.y;
      A(1, 2) = pow(ps.x, 2) + pow(ps.y, 2);
      A(1, 3) = 1;
      A(2, 0) = e.two.x;
      A(2, 1) = e.two.y;
      A(2, 2) = pow(e.two.x, 2) + pow(e.two.y, 2);
      A(2, 3) = 1;
      A(3, 0) = LIST[i].x;
      A(3, 1) = LIST[i].y;
      A(3, 2) = pow(LIST[i].x, 2) + pow(LIST[i].y, 2);
      A(3, 3) = 1;
      det = A.determinant();
      if (det > 0) {
        return false;
      }

    } else {
      A(0, 0) = e.one.x;
      A(0, 1) = e.one.y;
      A(0, 2) = pow(e.one.x, 2) + pow(e.one.y, 2);
      A(0, 3) = 1;
      A(1, 0) = e.two.x;
      A(1, 1) = e.two.y;
      A(1, 2) = pow(e.two.x, 2) + pow(e.two.y, 2);
      A(1, 3) = 1;
      A(2, 0) = ps.x;
      A(2, 1) = ps.y;
      A(2, 2) = pow(ps.x, 2) + pow(ps.y, 2);
      A(2, 3) = 1;
      A(3, 0) = LIST[i].x;
      A(3, 1) = LIST[i].y;
      A(3, 2) = pow(LIST[i].x, 2) + pow(LIST[i].y, 2);
      A(3, 3) = 1;
      det = A.determinant();
      if (det > 0) {
        return false;
      }
    }
  }
  return true;
}

float dd(Edge e, Point p) {
  float n_ab, n_ac, n_bc, circumradius;


  // build edge vector
  Vector2f ab = {e.one.x - e.two.x, e.one.y - e.two.y};
  Vector2f ac = {e.one.x - p.x, e.one.y - p.y};
  Vector2f bc = {p.x - e.two.x, p.y - e.two.y};


  // compute norm
  n_ab = ab.norm();
  n_ac = ac.norm();
  n_bc = bc.norm();


  //compute circumradius
  circumradius =
      (n_ab * n_ac * n_bc) / sqrt((n_ab + n_ac + n_bc) * (n_ac + n_bc - n_ab) *
                                  (n_bc + n_ab - n_ac) * (n_ab + n_ac - n_bc));


  // Outer space
  pdd A = make_pair(e.one.x, e.one.y);
  pdd B = make_pair(e.two.x, e.two.y);
  pdd C = make_pair(p.x, p.y);

  Point c = findCircumCenter(A, B, C);
  Point median_e = Point((e.one.x + e.two.x) / 2, (e.one.y + e.two.y) / 2);
  Vector2f vect_med_center = {median_e.x - c.x, median_e.y - c.y};
  Vector2f vect_med_p = {median_e.x - p.x, median_e.y - p.y};

  if (vect_med_center.dot(vect_med_p) < 0) {
    return -circumradius;
  }
  return circumradius;
}



float dd2(Edge e, Point p) {
  float n_ab, n_ac, n_bc, circumradius;
  float max_coord_x, min_coord_x, max_coord_y, min_coord_y;
  Vector2f ab = {e.one.x - e.two.x, e.one.y - e.two.y};
  Vector2f ac = {e.one.x - p.x, e.one.y - p.y};
  Vector2f bc = {p.x - e.two.x, p.y - e.two.y};

  n_ab = ab.norm();
  n_ac = ac.norm();
  n_bc = bc.norm();
  if (n_ab == 0 || n_ac == 0 || n_bc == 0) {
    return MAXFLOAT;
  }

  circumradius =
      (n_ab * n_ac * n_bc) / sqrt((n_ab + n_ac + n_bc) * (n_ac + n_bc - n_ab) *
                                  (n_bc + n_ab - n_ac) * (n_ab + n_ac - n_bc));
  MatrixXf A(3, 3);
  MatrixXf B(3, 3);
  A(0, 0) = e.one.x;
  A(0, 1) = e.one.y;
  A(0, 2) = 1;
  A(1, 0) = e.two.x;
  A(1, 1) = e.two.y;
  A(1, 2) = 1;
  A(2, 0) = p.x;
  A(2, 1) = p.y;
  A(2, 2) = 1;
  float a = A.determinant();
  B(0, 0) = pow(A(0, 0), 2) + pow(A(0, 1), 2);
  B(0, 1) = A(0, 1);
  B(1, 0) = pow(A(1, 0), 2) + pow(A(1, 1), 2);
  B(1, 1) = A(1, 1);
  B(2, 0) = pow(A(2, 0), 2) + pow(A(2, 1), 2);
  B(2, 1) = A(2, 1);
  B(0, 2) = 1;
  B(1, 2) = 1;
  B(2, 2) = 1;
  MatrixXf Bp;
  Bp = B;
  Bp(0, 1) = A(0, 0);
  Bp(1, 1) = A(1, 0);
  Bp(2, 1) = A(2, 0);
  float b1 = B.determinant();
  float b2 = Bp.determinant();
  Vector2f C = {-b1 / (2 * a), -b2 / (2 * a)};
  float coord_y[] = {A(0, 1), A(1, 1), A(2, 1)};
  float coord_x[] = {A(1, 0), A(1, 0), A(2, 0)};
  min_coord_y = *min_element(coord_y, coord_y + 3);
  max_coord_y = *max_element(coord_y, coord_y + 3);
  max_coord_x = *max_element(coord_x, coord_x + 3);
  min_coord_x = *min_element(coord_x, coord_x + 3);

  pdd A1 = make_pair(e.one.x, e.one.y);
  pdd B1 = make_pair(e.two.x, e.two.y);
  pdd C1 = make_pair(p.x, p.y);

  Point c = findCircumCenter(A1, B1, C1);
  Point median_e = Point((e.one.x + e.two.x) / 2, (e.one.y + e.two.y) / 2);
  Vector2f vect_med_center = {median_e.x - c.x, median_e.y - c.y};
  Vector2f vect_med_p = {median_e.x - p.x, median_e.y - p.y};

  if (vect_med_center.dot(vect_med_p) < 0) {
    return -circumradius;
  }
  return circumradius;

  if (C(0) > min_coord_x and C(0) < max_coord_x and C(1) > min_coord_y and
      C(1) < max_coord_y) {
    return circumradius;
  }
  return circumradius;
}

vector<Edge> point_vect_to_vect_edge(vector<Point> &ps) {
  vector<Edge> edges;
  for (int i = 1; i < ps.size(); i++) {
    edges.push_back(Edge(ps[i - 1], ps[i]));
  }
  return edges;
}

bool not_in_vect(vector<Point> vect_point, Point p) {
  for (int i = 0; i < vect_point.size(); i++) {
    if (p == vect_point[i]) {
      return false;
    }
  }
  return true;
}

// GRAPHIC

void Meshing::draw_point(int x, int y, sf::Color color) {
  sf::RectangleShape s{sf::Vector2f(4, 4)};
  s.setPosition(static_cast<float>(x), static_cast<float>(y));
  s.setFillColor(color);
  window->draw(s);
}
void Meshing::draw_points(vector<Point> p, sf::Color color) {
  for (int i = 0; i < p.size(); i++) {
    draw_point(p[i].x, p[i].y, color);
  }
}

void Meshing::draw_points() {
  for (int i = 0; i < points.size(); i++) {
    draw_point(points[i].x, points[i].y);
  }
}

void Meshing::draw_line(float x1, float y1, float x2, float y2,
                        sf::Color color) {
  sf::Vertex line[2];
  line[0].position = sf::Vector2f(x1, y1);
  line[0].color = color;
  line[1].position = sf::Vector2f(x2, y2);
  line[1].color = color;
  window->draw(line, 2, sf::Lines);
}

void Meshing::draw_triangle(Triangle t, sf::Color color) {
  draw_line(t.two.x, t.two.y, t.three.x, t.three.y, color);
  draw_line(t.one.x, t.one.y, t.three.x, t.three.y, color);
}