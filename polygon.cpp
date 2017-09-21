
//------------------------------------------------------------------------------
//  Copyright 2007-2017 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#include <vector>
#include "polygon.h"
#include "intersection.h"

string VertexTypeName[] = {"START", "END", "SPLIT", "MERGE", "REGULAR_DOWN", "REGULAR_UP", "UNKNOWN" };

ply_vertex::~ply_vertex()
{
	//doing nothing for now
}

bool ply_vertex::operator==(const ply_vertex& v2)
{
	return this->getVID() == v2.getVID() && this->getPos() == v2.getPos();
}

bool ply_vertex::operator>(const ply_vertex& v2)
{
	Point2d p1 = this->getPos();
	Point2d p2 = v2.getPos();

	return ((p1[1] > p2[1]) || ((p1[1] == p2[1]) && (p1[0] < p2[0])));
}

bool ply_vertex::operator<(const ply_vertex& v2)
{
	Point2d p1 = this->getPos();
	Point2d p2 = v2.getPos();

	return ((p1[1] < p2[1]) || ((p1[1] == p2[1]) && (p1[0] > p2[0])));
}

bool ply_vertex::operator!=(const ply_vertex& v2)
{
	return !(*this == v2);
}

// - compute normal
// - check if the vertex is reflex or not
void ply_vertex::computeExtraInfo()
{
	//compute normal direction
	Vector2d v=next->pos-pos;
	if( v[0]==0 ){
		if(v[1]>0){ normal[0]=1; normal[1]=0; }
		else{ normal[0]=-1; normal[1]=0; }
	}
	else if( v[0]>0 ){
		normal[1]=-1;
		normal[0]=(v[1]/v[0]);
	}
	else{//v[0]<0
		normal[1]=1;
		normal[0]=-(v[1]/v[0]);
	}
	normal=normal.normalize();

	//compute if left or right turn
	Vector2d u=pos-pre->pos;
	float z=u[0]*v[1]-u[1]*v[0];

	if(z<=0) reflex=true;
	else reflex=false;
}

void ply_vertex::negate()
{
	normal=-normal;
	pos[0]=-pos[0];
	pos[1]=-pos[1];
}

void ply_vertex::reverse()
{
	swap(next,pre);
	//normal=-normal;
	computeExtraInfo();
}

void ply_vertex::copy(ply_vertex * other)
{
	pos=other->pos;
	normal=other->normal;
	reflex=other->reflex;
	vid=other->vid;
}

void ply_vertex::rotate(double r)
{
	double cos_r=cos(r);
	double sin_r=sin(r);
	//rotate pos
	double x=pos[0]*cos_r-pos[1]*sin_r;
	double y=pos[0]*sin_r+pos[1]*cos_r;
	pos.set(x,y);

	//rotate normal
	x=normal[0]*cos_r-normal[1]*sin_r;
	y=normal[0]*sin_r+normal[1]*cos_r;
	normal.set(x,y);
}

ply_vertex::VertexType ply_vertex::getType()
{
	if(this->type != ply_vertex::UNKNOWN) return this->type;

	ply_vertex * prev_vertex = this->getPre();
	ply_vertex * next_vertex = this->getNext();

	if(*this > *next_vertex && *prev_vertex > *this)
		this->type = ply_vertex::REGULAR_DOWN;
	else if(*this > *prev_vertex && *next_vertex > *this)
		this->type = ply_vertex::REGULAR_UP;
	else if(*prev_vertex > *this && *next_vertex > *this)
	{
		if(this->isReflex())
			this->type  = ply_vertex::MERGE;
		else
			this->type = ply_vertex::END;

	}
	else if(*this > *prev_vertex && *this > *next_vertex)
	{
		if(this->isReflex())
			this->type = ply_vertex::SPLIT;
		else
			this->type = ply_vertex::START;
	}
	else
	{
		this->type = ply_vertex::UNKNOWN;
	}

	return this->type;
}

bool ply_vertex::isGoUp()
{
	ply_vertex* next = this->getNext();
	return this->getPos()[1] > next->getPos()[1];
}

bool ply_vertex::isGoDown()
{
	ply_vertex* next = this->getNext();
	return this->getPos()[1] <= next->getPos()[1];
}

string ply_vertex::getTypeName()
{
	return VertexTypeName[this->type];
}

///////////////////////////////////////////////////////////////////////////////

//copy from the given ply
void c_ply::copy(const c_ply& other)
{
	destroy();//detroy myself first

	ply_vertex* ptr=other.head;
	beginPoly();
	do{
		ply_vertex * v=new ply_vertex();
		assert(v); //check for memory
		v->copy(ptr);
		addVertex(v);
		ptr=ptr->getNext();
	}while( ptr!=other.head );

	//endPoly();
	//finish up
	tail->setNext(head);

	//copy extra info
	area=other.area;
	center=other.center;
	radius=other.radius;
	type=other.type;
}

// clean up the space allocated
void c_ply::destroy()
{
	if( head==NULL ) return;
	ply_vertex* ptr=head;
	do{
		ply_vertex * n=ptr->getNext();
		delete ptr;
		ptr=n;
	}while( ptr!=head );
	head=tail=NULL;

	all.clear();
}

// Create a empty polygon
void c_ply::beginPoly()
{
	head=tail=NULL;
	all.clear();
}

// Add a vertex to the polygonal chain
void c_ply::addVertex( double x, double y, int vid, bool remove_duplicate )
{
	Point2d pt(x,y);

	if(tail!=NULL){
		if(tail->getPos()==pt && remove_duplicate) return; //don't add
	}

	ply_vertex * v=new ply_vertex(pt);
	if( tail!=NULL ){
		tail->setNext(v);
	}
	tail=v;
	if( head==NULL ) head=tail;
	if(vid == -1)
		v->setVID(all.size()); //id of the vertex in this ply
	else
		v->setVID(vid);
	all.push_back(v);

}

// Add a vertex to the polygonal chian
void c_ply::addVertex( ply_vertex * v )
{
	if( tail!=NULL ){
		tail->setNext(v);
	}
	tail=v;
	if( head==NULL ) head=tail;
	v->setVID(all.size()); //id of the vertex in this ply
	all.push_back(v);
}

// finish building the polygon
void c_ply::endPoly(bool remove_duplicate)
{
	if(head!=NULL && tail!=NULL){
		if(remove_duplicate){
			if(head->getPos()==tail->getPos()){ //remove tail..
				delete tail;
				all.pop_back();
				tail=all.back();
			}
		}//
	}

	tail->setNext(head);
	doInit();
}

// initialize property of the this polychain
// Compute normals and find reflective vertices
void c_ply::doInit()
{
	//compute area
	getArea();
	if(this->area<0 && type==POUT){
#if VERBOSE
	   cerr<<"! Warning: polygon type is POUT but has negative area. Reverse the vertex ordering."<<endl;
#endif
	   reverse();
	}
	else if(this->area>0 && type==PIN){
#if VERBOSE
	   cerr<<"! Warning: polygon type is PIN but has positive area. Reverse the vertex ordering."<<endl;
#endif
	   reverse();
	}

	//compute normals
	ply_vertex* ptr=head;
	do{
		ptr->computeExtraInfo();
		ptr=ptr->getNext();
	}while( ptr!=head );
}

const Point2d& c_ply::getCenter()
{
	if(radius<0){
		center.set(0,0);
		ply_vertex * ptr=head;
		const Point2d& first=ptr->getPos();
		uint size=0;
		do{
			size++;
			Vector2d v=ptr->getPos()-first;
			center[0]+=v[0];
			center[1]+=v[1];
			ptr=ptr->getNext();
		}while(ptr!=head); //end while
		center[0]=(center[0]/size)+first[0];
		center[1]=(center[1]/size)+first[1];

		radius=0;
	}

	return center;
}


///////////////////////////////////////////////////////////////////////////
void c_ply::negate()
{
	ply_vertex * ptr=head;
	do{
		ptr->negate();
		ptr=ptr->getNext();
	}while(ptr!=head); //end while
}

//reverse the order of the vertices
void c_ply::reverse()
{
	ply_vertex * ptr=head;
	do{
		ptr->reverse();
		ptr=ptr->getNext();
	}
	while(ptr!=head); //end while

	this->area=-this->area;
	all.clear();
}

///////////////////////////////////////////////////////////////////////////
void c_ply::translate(const Vector2d& v)
{
	ply_vertex * ptr=head;
	do{
		ptr->translate(v);
		ptr=ptr->getNext();
	}while(ptr!=head); //end while
}

void c_ply::rotate(double radius)
{
	ply_vertex * ptr=head;
	do{
		ptr->rotate(radius);
		ptr=ptr->getNext();
	}while(ptr!=head); //end while
}


///////////////////////////////////////////////////////////////////////////
//compute the Radius of the poly chain
float c_ply::getRadius()
{
	if(radius<0) getCenter();

	if(radius==0){
		ply_vertex * ptr=head;
		do{
			float d=(center-ptr->getPos()).normsqr();
			if(d>radius) radius=d;
			ptr=ptr->getNext();
		}while(ptr!=head); //end while
		radius=sqrt(radius);
	}

	return radius;
}


float c_ply::getArea()
{
	if(area==-FLT_MAX){
		area=0;
		getCenter();
		ply_vertex * ptr=head;
		do{
			ply_vertex * next=ptr->getNext();
			const Point2d& p1=ptr->getPos();
			const Point2d& p2=next->getPos();
			area+=Area(p1.get(),p2.get(),center.get());
			ptr=next;
		}while(ptr!=head); //end while
	}

	return area;
}

//check if convex
bool c_ply::is_convex() const
{
	ply_vertex * ptr=head;
	do{
		if(ptr->isReflex()) return false;
		ptr=ptr->getNext();
	}while(ptr!=head); //end while

	return true;
}

void c_ply::delete_vertex(ply_vertex * v)
{
	ply_vertex *pre=v->getPre();
	ply_vertex *next=v->getNext();

	pre->setNext(next);
	next->setPre(pre);
	pre->computeExtraInfo(); //recompute info

	if(head==v){
		head=next;
		tail=pre;
	}
	delete v;

	all.clear(); //not valid anymore
}

void c_ply::build_all()
{
	uint vid=0;
	all.clear();
	ply_vertex * ptr=head;
	do{
		ptr->setVID(vid++);
		all.push_back(ptr);
		ptr=ptr->getNext();
	}while(ptr!=head); //end while
}

//
// Compute the center and the box of a list of plys
//

void c_plylist::buildBoxAndCenter()
{
	//typedef list<c_ply>::iterator IT;
	box[0]=box[2]=FLT_MAX;
	box[1]=box[3]=-FLT_MAX;
	for(iterator i=begin();i!=end();i++){

		ply_vertex * ptr=i->getHead();
		do{
			const Point2d& p=ptr->getPos();
			if(p[0]<box[0]) box[0]=p[0];
			if(p[0]>box[1]) box[1]=p[0];
			if(p[1]<box[2]) box[2]=p[1];
			if(p[1]>box[3]) box[3]=p[1];
			ptr=ptr->getNext();
		}
		while(ptr!=i->getHead()); //end while
	}

	center[0]=(box[0]+box[1])/2;
	center[1]=(box[2]+box[3])/2;

	is_buildboxandcenter_called=true;
}

//
// Compute the center and the box of a list of plys
//
void c_plylist::translate(const Vector2d& v)
{
	for(iterator i=begin();i!=end();i++) i->translate(v);
}

//
// rotate all polychains
//
void c_plylist::rotate(double r)
{
	for(iterator i=begin();i!=end();i++) i->rotate(r);
}

void c_plylist::negate()
{
	for(iterator i=begin();i!=end();i++) i->negate();
}

void c_polygon::reverse()
{
	for(iterator i=begin();i!=end();i++) i->reverse();
	all.clear();
}

bool c_polygon::valid() //check if this is a valid polygon
{
	typedef list<c_ply>::iterator IT;
	if(empty()) return false;
	if(front().getType()!=c_ply::POUT) return false;
	for(iterator i=++begin();i!=end();i++) if(i->getType()!=c_ply::PIN) return false;

	return true;
}

//copy from the given polygon
void c_polygon::copy(const c_polygon& other)
{
	clear();
	all.clear();

	for(const_iterator i=other.begin();i!=other.end();i++){
		c_ply p(c_ply::UNKNOWN);
		p.copy(*i);
		push_back(p);
	}
}

list<c_polygon> c_polygon::split()
{
	list<c_polygon> sub;
	for(iterator i=begin();i!=end();i++){
		if(i->getType()==c_ply::POUT){
			c_polygon tmp;
			tmp.push_back(*i);
			sub.push_back(tmp);
		}
		else{
			if(sub.empty()){
				cerr<<"! Error: Invalid polygon type: Holes are defined without external boundary."<<endl;
				continue; //can't do anything about this...
			}
			sub.back().push_back(*i);
		}
	}
	return sub;
}

void c_polygon::destroy()
{
	for(iterator i=begin();i!=end();i++){
		i->destroy();
	}
	clear(); //remove all ply from this list
	all.clear();
}

bool c_polygon::is_convex() const
{
	if(size()>1) return false; //contains hole
	return front().is_convex();
}

void c_polygon::build_all()
{
	uint vid=0;
	for(iterator i=begin();i!=end();i++){
		uint vsize=i->getSize();
		for(uint j=0;j<vsize;j++){
			if((*i)[j]->getVID() == UINT_MAX) //XXX this implementation has some issues
				(*i)[j]->setVID(vid++);       //id of vertex in the polygons
			this->vid_index_map[(*i)[j]->getVID()] = all.size();
			all.push_back((*i)[j]);
		}
	}//end for i
}

ply_vertex* c_polygon::getVertexByVID(uint vid)
{
	return all[this->vid_index_map[vid]];
}

double c_polygon::getArea()
{
	if(area==0){
		for(iterator i=begin();i!=end();i++){
			area+=i->getArea();
		}//end for i
	}

	return area;
}

istream& operator>>( istream& is, c_ply& poly)
{
	int vsize; string str_type;
	is>>vsize>>str_type;

	if( str_type.find("out")!=string::npos )
		poly.type=c_ply::POUT;
	else poly.type=c_ply::PIN;

	poly.beginPoly();
	//read in all the vertices
	int iv;
	vector< pair<double,double> > pts; pts.reserve(vsize);
	for( iv=0;iv<vsize;iv++ ){
		double x,y;
		is>>x>>y;
		pts.push_back(pair<double,double>(x,y));
		//double d=x*x+y*y;
	}
	int id;
	for( iv=0;iv<vsize;iv++ ){
		is>>id; id=id-1;
		poly.addVertex(pts[id].first,pts[id].second);
	}

	poly.endPoly();
	return is;
}

istream& operator>>( istream& is, c_plylist& p)
{
	//remove header comments
	do{
		char tmp[1024];
		char c=is.peek();
		if(isspace(c)) is.get(c); //eat it
		else if(c=='#') {
			is.getline(tmp,1024);
		}
		else break;
	}while(true);

	//start reading
	uint size;
	is>>size;
	uint vid=0;
	for(uint i=0;i<size;i++){
		c_ply poly(c_ply::UNKNOWN);
		is>>poly;
		p.push_back(poly);
		uint vsize=poly.getSize();
		for(uint j=0;j<vsize;j++){
			poly[j]->setVID(vid++);       //id of vertex in the polygons
		}
	}
	return is;
}

ostream& operator<<( ostream& os, c_ply& p)
{
	os<<p.getSize()<<" "<<((p.type==c_ply::PIN)?"in":"out")<<"\n";
	ply_vertex * ptr=p.head;
	do{
		os<<ptr->getPos()[0]<<" "<<ptr->getPos()[1]<<"\n";
		ptr=ptr->getNext();
	}while(ptr!=p.head);

	for(int i=0;i<p.getSize();i++) os<<i+1<<" ";

	os<<"\n";

	return os;
}

ostream& operator<<( ostream& out, c_plylist& p)
{
	out<<p.size()<<"\n";
	typedef c_plylist::iterator PIT;
	for(PIT i=p.begin();i!=p.end();i++) out<<*i;
	return out;
}
