#define __CL_ENABLE_EXCEPTIONS

#include "meshing.hpp"
#include "hull.cpp"
#include "circumcenter.cpp"

using namespace std;
using namespace Eigen;



// pick up device type from compiler command line or from the default type
#ifndef DEVICE
#define DEVICE CL_DEVICE_TYPE_CPU
#endif


#define TOL    (0.001)   // tolerance used in floating point comparisons
#define LENGTH (1<<23)    // length of vectors a, b, and c

// TRIANGULATION

Meshing::Meshing(int witdh, int height, sf::RenderWindow  *win ){
    window = win;
    window->setFramerateLimit(1);
    MatrixXd points_eigen = MatrixXd::Random(200,2)*witdh;
    points_eigen = points_eigen.array().abs();
    
    for(int i = 0; i < points_eigen.rows(); i++){
        points.push_back(Point(points_eigen(i,0), points_eigen(i,1), i));
    }
}


// not functionnal
int Meshing::triangulation(int nb_partition){
    long delta = 0;

    vector<Point> H1, H2, path;
    vector<Edge> edge_path;
    path = partition(points, H1, H2);
    vector<Triangle>  triangle_list;
    edge_path = point_vect_to_vect_edge(path);
    for( int i = 0; i < nb_partition; i++){
        
    }
    // ParDeTri(H1, edge_path, triangle_list);
    // ParDeTri(H2, edge_path, triangle_list);


    for( int i = 0; i < triangle_list.size(); i++){
        // cout << "draw " << i << endl;
        cout << "draw " << i << endl;
        draw_triangle(triangle_list[i]);
    }
}

std::vector<Point> Meshing::partition(std::vector<Point> list_points, vector<Point> &H1, vector<Point> &H2){
    vector<Point> path;
    path = partition_path(list_points);
    int s;
    // fill partitions
    for(int i = 0; i < list_points.size(); i++){
        if ( ! std::count(path.begin(), path.end(), list_points[i])){
            s = side(list_points[i], path);
            if ( s < 0){
                draw_point(list_points[i].x, list_points[i].y, sf::Color::Green);
                H1.push_back(list_points[i]);

            }   
            if( s >= 0)
            {   
                draw_point(list_points[i].x, list_points[i].y, sf::Color::Blue);
                H2.push_back(list_points[i]);
            }
        }
    }
    for(int i = 0; i< path.size(); i++){
        H1.push_back(path[i]);
        H2.push_back(path[i]);
        draw_point(path[i].x, path[i].y, sf::Color::Yellow);

    }
    return path;
}


void Meshing::ParDeTri(vector<Point> point_set, vector<Edge> edge_list, vector<Triangle> &triangle_list){
    int index_nearest_point;
    int it = 0;
    int max_it = 200;
    while( edge_list.size() > 0 && it < max_it){
        it++;
        cout << "********* edge_list size : " << edge_list.size() << "*************" << endl;
        // pop first edge
        int i = 0;
        Edge e = Edge(edge_list[i].one, edge_list[i].two);
        edge_list.erase(edge_list.begin()+i);

        // make a delaunay triangle
        index_nearest_point = nearest_point(point_set, e);
        cout << "nearest point :" << index_nearest_point << endl;
        Triangle t = Triangle(e, point_set[index_nearest_point]);

        Edge ep = Edge(e.one, point_set[index_nearest_point]);
        Edge ep2 = Edge(e.two, point_set[index_nearest_point]);
        // point_set.erase(point_set.begin()+index_nearest_point);
        // Update
        if(t.is_triangle()){
            triangle_list.push_back(t);
            update(ep, edge_list);
            update(ep2, edge_list);
        }
        else
            cout <<  "not triangle" << endl;
    }
    cout <<  "finished" << endl;
    return;
}

void update(Edge e, vector<Edge> &edge_list){
    bool find = false;
    for( int i = 0; i < edge_list.size(); i++){   
        if(e == edge_list[i]){
            edge_list.erase(edge_list.begin()+i);
            cout << "#### erase " << i << endl;
            return;
        }
    }
    cout << "#### push back  " << e.one.index <<  endl;
    edge_list.push_back(e);
}


// TODO choose vertical/horizontale
// Now suit with verticale path
int Meshing::side(Point p, vector<Point> &path){
    // find segment 
    for(int i = 1; i < path.size(); i++){

        if( p.y <= path[i-1].y && p.y > path[i].y){
            return  (p.x - path[i-1].x) * (path[i].y - path[i-1].y) -  (p.y- path[i-1].y) * (path[i].x -path[i-1].x);
        }
    }
    if( p.y <= path[path.size()-1].y && p.y > path[0].y  && path[path.size()-1].y > path[0].y){
        return  (p.x - path[path.size()-1].x) * (path[0].y - path[path.size()-1].y) -  (p.y- path[path.size()-1].y) * (path[0].x -path[path.size()-1].x);
    }
    return -1;
}



//TODO choose between x or y median
vector<Point> Meshing::partition_path(std::vector<Point> &list_points){
    // copy vector
    vector<Point> proj;
    // find median
    std::vector<float> xcoord;
    for (int i=0; i<list_points.size(); i++){
        xcoord.push_back(list_points[i].x);
     }
    int n = sizeof(xcoord)/sizeof(xcoord[0]); 
    sort(xcoord.begin(),  xcoord.end());
    float median  = xcoord[xcoord.size()/2];
    
    //3d projection
    vector<Point> hull;
    for(int i = 0; i < list_points.size(); i++){
        proj.push_back(Point(list_points[i].y,pow(list_points[i].x-median, 2) + pow(list_points[i].y,2), list_points[i].index));
    }
    // delauney path :
    hull = quickHull(proj);
    vector<Point> path;
    int index, index2;
    for (int i = 0; i < hull.size()-1; i++ ){
        index = hull[i].index;
        index2 = hull[i+1].index;
        if(list_points[index].y > list_points[index2].y){
            draw_line(list_points[index].x, list_points[index].y, list_points[index2].x, list_points[index2].y, sf::Color::Red);
            path.push_back(list_points[index]);
        }
    }
    index = hull[hull.size()-1].index;
    index2 = hull[0].index;
    if(list_points[index].y > list_points[index2].y){
        draw_line(list_points[index].x, list_points[index].y, list_points[index2].x, list_points[index2].y, sf::Color::Red);
        path.push_back(list_points[index]);
    }        
    
    
    path.push_back(list_points[index2]);
    return path;
}


int Meshing::nearest_point_gpu(vector<Point> &ps, Edge &e){

    // cl::Buffer p_x;                        // device memory used for the input  a vector
    // cl::Buffer p_y;                       // device memory used for the input  b vector          
    return 1;         

}

int Meshing::nearest_point(vector<Point> &ps, Edge &e){ 
    float min = MAXFLOAT;
    float dis, min_dis;
    int index_min = -1;
    for( int i = 0; i < ps.size(); i++){
        dis = dd2(e, ps[i]);
        // cout << "dis " << dis << " " << i << endl;
        if( min > dis ){
            min = dis;
            index_min  = i;
            min_dis = dis;
        }   
    }
    // cout << "dis" << index_min << " "  << min_dis << endl;
    return index_min;
}

bool Meshing::is_goodtriangle(vector<Point> &LIST, Point &ps, Edge &e){
    MatrixXf A(4,4);
    float det;
    for( int i = 0; i < LIST.size(); i++){
        if(((e.one.x*e.two.y - e.two.x*e.one.y) +(e.two.x*ps.y - ps.x*e.two.y) + (ps.x*e.one.y - e.one.x*ps.y))<0){
            A(0,0) = e.one.x;
            A(0,1) = e.one.y;
            A(0,2) = pow(e.one.x,2) + pow(e.one.y, 2);
            A(0,3) = 1 ;
            A(1,0) = ps.x; 
            A(1,1) = ps.y;
            A(1,2) = pow(ps.x, 2) + pow(ps.y, 2);
            A(1,3) = 1;
            A(2,0) = e.two.x; 
            A(2,1) = e.two.y;
            A(2,2) = pow(e.two.x,2) + pow(e.two.y, 2);
            A(2,3) = 1;
            A(3,0) = LIST[i].x;
            A(3,1) = LIST[i].y;
            A(3,2) = pow(LIST[i].x, 2) + pow(LIST[i].y, 2);
            A(3,3) = 1;
            det = A.determinant();
            if(det >0){
                return false;}
            
        }
        else{
            A(0,0) = e.one.x;
            A(0,1) = e.one.y;
            A(0,2) = pow(e.one.x,2) + pow(e.one.y, 2);
            A(0,3) = 1 ;
            A(1,0) = e.two.x;
            A(1,1) = e.two.y;
            A(1,2) = pow(e.two.x,2) + pow(e.two.y, 2);
            A(1,3) = 1;
            A(2,0) = ps.x;
            A(2,1) = ps.y;
            A(2,2) = pow(ps.x, 2) + pow(ps.y, 2);
            A(2,3) = 1;
            A(3,0) = LIST[i].x;
            A(3,1) = LIST[i].y;
            A(3,2) = pow(LIST[i].x, 2) + pow(LIST[i].y, 2);
            A(3,3) = 1;
            det = A.determinant();
            if(det >0){
                return false;}
        }

    }
    return true;
}

float dd(Edge e, Point p){
    float n_ab, n_ac, n_bc, circumradius;

    Vector2f ab = {e.one.x - e.two.x, e.one.y - e.two.y};
    Vector2f ac = {e.one.x - p.x, e.one.y - p.y};
    Vector2f bc = {p.x - e.two.x, p.y - e.two.y};
    
    n_ab = ab.norm();
    n_ac = ac.norm();
    n_bc = bc.norm();
    if (n_ab == 0 || n_ac == 0 || n_bc == 0){
        return MAXFLOAT;
    }

    circumradius = (n_ab * n_ac * n_bc)/ sqrt((n_ab+n_ac+n_bc)*(n_ac+n_bc-n_ab)*(n_bc+n_ab-n_ac)*(n_ab+n_ac-n_bc));

    float bac = acos(ab.dot(ac)/(n_ab*n_ac));
    float abc = acos(ab.dot(bc)/(n_ab*n_bc));
    float bca = acos(ac.dot(-bc)/(n_ac*n_bc));
    //

    pdd A = make_pair(e.one.x, e.one.y);
    pdd B = make_pair(e.two.x, e.two.y);
    pdd C = make_pair(p.x, p.y);

    Point c = findCircumCenter(A, B, C);
    // cout << " center " << c.x << " " << c.y << endl;
    Point median_e = Point((e.one.x + e.two.x)/2 , (e.one.y + e.two.y)/2);
    Vector2f vect_med_center = {median_e.x - c.x, median_e.y - c.y};
    Vector2f vect_med_p = {median_e.x - p.x, median_e.y - p.y};
    

    if(vect_med_center.dot(vect_med_p) < 0){
        return -circumradius;
    }
    return circumradius;

    // obtute ? -circumdius
    // cout << bac <<  abc <<  bca << endl;
    if (bac < 3.1415/2 && abc < 3.1415/2 && bca < 3.1415/2){
        return circumradius;
    }
     return circumradius;
}



float dd2(Edge e, Point p){
    float n_ab, n_ac, n_bc, circumradius;
    float max_coord_x, min_coord_x, max_coord_y, min_coord_y;
    Vector2f ab = {e.one.x - e.two.x, e.one.y - e.two.y};
    Vector2f ac = {e.one.x - p.x, e.one.y - p.y};
    Vector2f bc = {p.x - e.two.x, p.y - e.two.y};

    n_ab = ab.norm();
    n_ac = ac.norm();
    n_bc = bc.norm();
    if (n_ab == 0 || n_ac == 0 || n_bc == 0){
        return MAXFLOAT;
    }

    circumradius = (n_ab * n_ac * n_bc)/ sqrt((n_ab+n_ac+n_bc)*(n_ac+n_bc-n_ab)*(n_bc+n_ab-n_ac)*(n_ab+n_ac-n_bc));
    MatrixXf A(3,3);
    MatrixXf B(3,3);
    A(0,0) = e.one.x;
    A(0,1) = e.one.y;
    A(0,2) = 1;
    A(1,0) = e.two.x;
    A(1,1) = e.two.y;
    A(1,2) = 1;
    A(2,0) = p.x;
    A(2,1) = p.y;
    A(2,2) = 1;
    float a = A.determinant();
    B(0,0) = pow(A(0,0),2) + pow(A(0,1),2);
    B(0,1) = A(0,1);
    B(1,0) = pow(A(1,0),2) + pow(A(1,1),2);
    B(1,1) = A(1,1);
    B(2,0) = pow(A(2,0),2) + pow(A(2,1),2);
    B(2,1) = A(2,1);
    B(0,2) = 1;
    B(1,2) = 1;
    B(2,2) = 1;
    MatrixXf Bp;
    Bp = B;
    Bp(0,1) = A(0,0);
    Bp(1,1) = A(1,0);
    Bp(2,1) = A(2,0);
    float b1 = B.determinant();
    float b2 = Bp.determinant();
    Vector2f C = {-b1/(2*a), -b2/(2*a)};
    float coord_y[] = {A(0,1), A(1,1), A(2,1)};
    float coord_x[] = {A(1,0), A(1,0), A(2,0)};
    min_coord_y = *min_element(coord_y, coord_y + 3);
    max_coord_y = *max_element(coord_y, coord_y + 3);
    max_coord_x = *max_element(coord_x, coord_x + 3);
    min_coord_x = *min_element(coord_x, coord_x + 3);



    pdd A1 = make_pair(e.one.x, e.one.y);
    pdd B1 = make_pair(e.two.x, e.two.y);
    pdd C1 = make_pair(p.x, p.y);

    Point c = findCircumCenter(A1, B1, C1);
    // cout << " center " << c.x << " " << c.y << endl;
    Point median_e = Point((e.one.x + e.two.x)/2 , (e.one.y + e.two.y)/2);
    Vector2f vect_med_center = {median_e.x - c.x, median_e.y - c.y};
    Vector2f vect_med_p = {median_e.x - p.x, median_e.y - p.y};
    

    if(vect_med_center.dot(vect_med_p) < 0){
        return -circumradius;
    }
    return circumradius;

    if(C(0) > min_coord_x and C(0) < max_coord_x  and C(1) > min_coord_y and C(1)< max_coord_y){
      return circumradius;
    }
    return circumradius;
}

vector<Edge> point_vect_to_vect_edge(vector<Point> &ps){
    vector<Edge> edges;
    for(int i = 1; i < ps.size(); i++){
        edges.push_back(Edge(ps[i-1], ps[i]));
    }
    return edges;
}

// GRAPHIC

void Meshing::draw_point(int x, int y, sf::Color color){
    sf::RectangleShape s{sf::Vector2f(4, 4)};
    s.setPosition(static_cast<float>(x), static_cast<float>(y));
    s.setFillColor(color);
    window->draw(s);
}
void Meshing::draw_points(vector<Point> p, sf::Color color){
    for(int i = 0; i < p.size(); i++){
        draw_point(p[i].x, p[i].y, color);
    }
}

void Meshing::draw_points(){
    for(int i = 0; i < points.size(); i++){
        draw_point(points[i].x, points[i].y);
    }
}

void Meshing::draw_line(float x1, float y1, float x2, float y2, sf::Color color){
    sf::Vertex line[2];
    line[0].position = sf::Vector2f(x1, y1);
    line[0].color  = color;
    line[1].position = sf::Vector2f(x2, y2);
    line[1].color = color;
    window->draw(line, 2, sf::Lines);

}

void Meshing::draw_triangle( Triangle t, sf::Color color){
    // draw_line(t.one.x, t.one.y, t.two.x, t.two.y, color);
    draw_line(t.two.x, t.two.y, t.three.x, t.three.y, color);
    draw_line(t.one.x, t.one.y, t.three.x, t.three.y, color);


}


int points_to_matrix(vector<Point> vect_points){
    for( int i = 0; i < vect_points.size(); i++){
        std:cout << i << std::endl;
    }
    return 1;
}

