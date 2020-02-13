#include "meshing.hpp"
#include "hull.cpp"

using namespace std;
using namespace Eigen;


// TRIANGULATION




Meshing::Meshing(int witdh, int height, sf::RenderWindow  *win ){
    window = win;
    window->setFramerateLimit(1);
    MatrixXd points_eigen = MatrixXd::Random(30,2)*witdh;
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
    // #pragma omp parallel
    // {
    //     ParDeTri(H1, edge_path, triangle_list);
    // }
    // #pragma omp parallel 
    // {
    //     ParDeTri(H2, edge_path, triangle_list);
    // }
    ParDeTri(H1, point_vect_to_vect_edge(path), triangle_list);
    ParDeTri(H2, point_vect_to_vect_edge(path), triangle_list);

    for( int i = 0; i < triangle_list.size(); i++){
        cout << "draw " << i << endl;
        cout << triangle_list[i].one.index << " " << triangle_list[i].two.index << " "  << triangle_list[i].three.index << " " << endl;
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
    int max_it = 100;
    while( edge_list.size() > 0 && it < max_it){
        it++;
        cout << "********* edge_list size : " << edge_list.size() << "*************" << endl;
        // pop first edge
        Edge e = Edge(edge_list[0].one, edge_list[0].two);
        edge_list.erase(edge_list.begin());

        // make a delaunay triangle
        index_nearest_point = nearest_point(point_set, e);
        cout << "nearest point :" << index_nearest_point << endl;
        Triangle t = Triangle(e, point_set[index_nearest_point]);

        // Update
        if(t.is_triangle()){
            triangle_list.push_back(t);
            update(Edge(t.one, t.three), edge_list);
            update(Edge(t.two, t.three), edge_list);
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
            find = true;
        }
    }
    if(find == false){
        cout << "#### push back  " << e.one.index <<  endl;
        edge_list.push_back(e);
    }

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


int Meshing::nearest_point(vector<Point> &ps, Edge &e){
    float min = MAXFLOAT;
    float dis, min_dis;
    int index_min = -1;
    for( int i = 0; i < ps.size(); i++){
        dis = dd(e, ps[i]);
        // cout << "dis " << dis << " " << i << endl;
        if( min > dis){
            min = dis;
            index_min  = i;
            min_dis = dis;
        }   
    }
    // cout << "dis" << index_min << " "  << min_dis << endl;
    return index_min;
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
    
    // obtute ? -circumdius
    // cout << bac + abc + bca << endl;
    if (bac < 3.1415/2 && abc < 3.1415/2 && bca < 3.1415/2){
        return circumradius;
    }
    return -circumradius;
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

