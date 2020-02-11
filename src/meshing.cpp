#include "meshing.hpp"
#include "hull.cpp"

using namespace std;
using namespace Eigen;


// TRIANGULATION

Meshing::Meshing(int witdh, int height, sf::RenderWindow  *win ){
    window = win;
    window->setFramerateLimit(1);
    MatrixXd points_eigen = MatrixXd::Random(100,2)*witdh;
    points_eigen = points_eigen.array().abs();

    for(int i = 0; i < points_eigen.rows(); i++){
        points.push_back(Point(points_eigen(i,0), points_eigen(i,1), i));
    }
}


int Meshing::partition_1(){
    vector<Point> path;
    vector<int> H1,H2;
    path = partition_path();
    int s;
    // fill partitions
    for(int i = 1; i < points.size(); i++){
        if ( ! std::count(path.begin(), path.end(), points[i])){
            s = side(points[i], path);
            if ( s < 0){
                draw_point(points[i].x, points[i].y, sf::Color::Green);
                H1.push_back(i);

            }   
            if( s > 0)
            {   
                draw_point(points[i].x, points[i].y, sf::Color::Blue);
                H2.push_back(i);
            }
        }
    }

    vector<Triangle>  triangle_listH1;
    triangle_listH1 = ParDeTri(H1, point_vect_to_vect_edge(path));
    for( int i = 0; i < triangle_listH1.size(); i++){
        cout << "draw " << i << endl;
        draw_triangle(triangle_listH1[i]);
    }
    return 1;
}


vector<Triangle> Meshing::ParDeTri(vector<int> H, vector<Edge> path){
    vector<Triangle> triangle_list;
    int index_nearest_point;
    Triangle t();
    while( path.size() > 0){
        // pop first edge
        Edge e = Edge(path[0].one, path[0].two, path[0].index);
        path.erase(path.begin());
        // make a delaunay triangle
        index_nearest_point = nearest_point(H, e);
        if(index_nearest_point == -1){
            return triangle_list;
        }
        cout << "nearest point :" << index_nearest_point << endl;
        Triangle t = Triangle(e, points[H[index_nearest_point]]);
        // draw_triangle(t);
        H.erase(H.begin()+index_nearest_point);
        triangle_list.push_back(t);

        if( path.size() > 0 ){
            // Update
            update(Edge(t.one, t.three), path);
            update(Edge(t.two, t.three), path);
        }
    }
    cout << " finished ParDeTri " << endl;
    return triangle_list;
}

void update(Edge e, vector<Edge> &path){
    for( int i = 0; i < path.size(); i++){   
        // cout << e.one.x << " " << e.one.y << " " << e.two.x << " " << e.two.y << endl;
        // cout << path[i].one.x << " " << path[i].one.y << " " << path[i].two.x << " " << path[i].two.y << endl;
        if(e == path[i]){
            path.erase(path.begin()+i);
            cout << "erase " << i << endl;
            return;
        }
    }
    cout << "push back  " << e.one.x<<  endl;
    path.push_back(e);
    return;
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
vector<Point> Meshing::partition_path(){
    // copy vector
    vector<Point> proj;
    // find median
    std::vector<float> xcoord;
    for (int i=0; i<points.size(); i++){
        xcoord.push_back(points[i].x);
     }
    int n = sizeof(xcoord)/sizeof(xcoord[0]); 
    sort(xcoord.begin(),  xcoord.end());
    float median  = xcoord[xcoord.size()/2];
    
    //3d projection
    vector<Point> hull;
    for(int i = 0; i < points.size(); i++){
        proj.push_back(Point(points[i].y,pow(points[i].x-median, 2) + pow(points[i].y,2), points[i].index));
    }
    // delauney path :
    hull = quickHull(proj);
    vector<Point> path;
    int index, index2;
    for(int j= 0; j<2; j++){
        for (int i = 0; i < hull.size()-1; i++ ){
            index = hull[i].index;
            index2 = hull[i+1].index;
            if(points[index].y > points[index2].y){
                draw_line(points[index].x, points[index].y, points[index2].x, points[index2].y, sf::Color::Red);
                path.push_back(points[index]);
            }
        }
        index = hull[hull.size()-1].index;
        index2 = hull[0].index;
        if(points[index].y > points[index2].y){
            draw_line(points[index].x, points[index].y, points[index2].x, points[index2].y, sf::Color::Red);
            path.push_back(points[index]);
        }        
    }
    
    path.push_back(points[index2]);
    return path;
}


int Meshing::nearest_point(vector<int> &ps, Edge &e){
    float min = MAXFLOAT;
    float dis;
    int index_min = -1;
    for( int i = 0; i < ps.size(); i++){
        dis = dd(e, points[ps[i]]);
        cout << "dis " << dis << endl;
        if( min > dis){
            min = dis;
            index_min  = i;
        }   
    }
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

    circumradius = (n_ab * n_ac * n_bc)/ sqrt(((n_ab+n_ac+n_bc)*(n_ac+n_bc-n_ab)*(n_bc+n_ab-n_ac)*(n_ab+n_ac-n_bc)));
    float bac = acos(-ab.dot(-ac)/(n_ab*n_ac));
    float abc = acos(-ab.dot(-bc)/(n_ab*n_bc));
    float bca = acos(-ac.dot(bc)/(n_ac*n_bc));
    
    // obtute ? -circumdius
    // cout << bac + abc + bca << endl;
    if (bac > 3.1415/2 || abc > 3.1415/2||bca > 3.1415/2){
        return -circumradius;
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


int main(int argc, char **argv){


    sf::RenderWindow window(sf::VideoMode(800, 800), "Delaunay triangulation");
    Meshing mesh = Meshing(800, 800, &window);
    mesh.draw_points();
    int fin = mesh.partition_1();

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
 