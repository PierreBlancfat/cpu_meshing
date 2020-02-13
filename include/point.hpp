#ifndef POINT_HPP
#define POINT_HPP



struct Point {
	int index;
	float x;
	float y;
	Point(float xIn, float yIn, int ind = -1) : x(xIn), y(yIn), index(ind){};
	bool operator==(Point a) const {
      if(a.x==x && a.y == y)
         return true;
      else
         return false;
}
};


struct Edge {
	int index;
	Point one, two;
	Edge(Point one, Point two, int ind = -1) : one(one), two(two), index(ind){};
	Edge(const Edge &e) : one(e.one), two(e.two), index(e.index){};
	Edge();
	bool operator==(Edge a) const {
      if(a.one==one && a.two == two || a.one==two && a.two == one){
         return true;
	  }
      else{
         return false;
	  }
}

};


struct Triangle {
	int index;
	Point one, two, three;
	Triangle(Point one, Point two,Point three, int ind = -1) : one(one), two(two),three(three), index(ind){};
	Triangle(Edge edge,Point point, int ind = -1) : one(edge.one), two(edge.two),three(point), index(ind){};
	Triangle();
	bool is_triangle(){
		if(one == two || two == three || one == three){
			return false;
		}
		return true;
	}
	bool operator==(Triangle t) const {
      if((t.one == one || t.two == one || t.three == one) && (t.one == two || t.two == two || t.three == two) && (t.one == three || t.two == three || t.three == three) ){
         return true;
	  }
      else{
         return false;
	  }
	}
};

#endif