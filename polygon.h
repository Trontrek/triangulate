//------------------------------------------------------------------------------
//  Copyright 2010-2017 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------
#pragma once
#ifndef _POLYGON_H_
#define _POLYGON_H_

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include "Basic.h"
#include "Point.h"
#include "Vector.h"
using namespace mathtool;

#include <list>
#include <cassert>
#include <vector>
#include <float.h>
#include <map>
using namespace std;

#include <limits.h>

typedef unsigned int uint;

//
// Vertex of polygon
//
class ply_vertex
{
public:
	enum VertexType { START, END, SPLIT, MERGE, REGULAR_DOWN, REGULAR_UP, UNKNOWN };
	///////////////////////////////////////////////////////////////////////////
	ply_vertex(){ init(); }
	ply_vertex( const Point2d& p ){ pos=p; init(); }
	virtual ~ply_vertex();
	void setNext(ply_vertex * n){next=n; if(n!=NULL) n->pre=this; }
	void setPre(ply_vertex * n){pre=n; if(n!=NULL) n->next=this; }
	void computeExtraInfo();

	//negate the vertex
	void negate();

	//reverse the order
	void reverse();

	//copy
	void copy(ply_vertex * other);

	///////////////////////////////////////////////////////////////////////////
	void setPos(const Point2d& p) { pos=p; }
	virtual const Point2d& getPos() const { return pos; }

	void translate(const Vector2d& v){ pos=pos+v; }

	void rotate(double r);

	virtual ply_vertex * getNext() const { return next; }
	virtual ply_vertex * getPre() const { return pre; }

	const Vector2d& getNormal() const { return normal; }
	bool isReflex() const { return reflex; }

	//get extra information
	uint getVID() const { return vid; }
	void setVID(uint id) {vid=id;}

	VertexType getType();
	string getTypeName();

	bool operator==(const ply_vertex&);
	bool operator>(const ply_vertex&);
	bool operator<(const ply_vertex&);
	bool operator!=(const ply_vertex&);

	bool isGoUp();
	bool isGoDown();

private:

	void init(){
		next=pre=NULL;
		reflex=false;
		vid=UINT_MAX;
		type=ply_vertex::UNKNOWN;
	}

	//basic info
	Point2d pos;       //position
	ply_vertex * next; //next vertex in the polygon
	ply_vertex * pre;  //previous vertex in the polygon
	Vector2d normal;   //normal, the segment normal from this v to the next.
	bool reflex;
	uint vid;
	enum VertexType type;	//type of vertex
};

class CompareVertex {
public:
    bool operator()(ply_vertex*& v1, ply_vertex*& v2)
    {
    	return (*v1) < (*v2);
    }
};

class ply_edge
{
public:
	ply_edge()
	{

	}
	ply_edge(ply_vertex* source, ply_vertex* target)
	{
		this->m_source = source;
		this->m_traget = target;
	}
	ply_vertex* getSource() { return this->m_source;}
	ply_vertex* getTarget() { return this->m_traget;}
	ply_vertex* getHelper() { return this->m_helper;}

	void setHelper(ply_vertex* helper) { this->m_helper = helper;}

	// key value is the left most x coordinate on the line or the extension of the edge intersected with line y=yKey
	double keyValue() { return this->m_keyValue; }
	void setKeyValue(double keyValue) {
		double x0 = m_source->getPos()[0];
		double x1 = m_traget->getPos()[0];
		double y0 = m_source->getPos()[1];
		double y1 = m_traget->getPos()[1];

		if(y0 == y1)
			this->m_keyValue = x0 < x1 ? x0 : x1;
		else
			this->m_keyValue = (keyValue - y0)*(x1 - x0) / (y1 - y0) + x0;
	}

	bool operator==( const ply_edge& other ){
		return ((m_source->getVID() == other.m_source->getVID()) && (m_traget->getVID() == other.m_source->getVID()));
	}

	bool operator>( const ply_edge& other){
		return m_keyValue > other.m_keyValue;
	}

	bool operator<( const ply_edge& other){
		return m_keyValue < other.m_keyValue;
	}

	void increaseKeyValue(const double diff) { m_keyValue+=diff; }

private:
	ply_vertex* m_source;
	ply_vertex* m_traget;
	double m_keyValue;
	ply_vertex* m_helper;
};

//
// Polygon chain
//
class c_ply{
public:

	enum POLYTYPE { UNKNOWN, PIN, POUT };

	///////////////////////////////////////////////////////////////////////////
	c_ply(POLYTYPE t){ head=tail=NULL; type=t; radius=-1; area=-FLT_MAX; }

	///////////////////////////////////////////////////////////////////////////
	void copy(const c_ply& ply); //copy from the other ply
	void destroy();

	///////////////////////////////////////////////////////////////////////////
	// create c_ply
	void beginPoly();
	void addVertex( double x, double y, int vid = -1, bool remove_duplicate=false );
	void addVertex( ply_vertex * v );
	void endPoly(bool remove_duplicate=false);

	///////////////////////////////////////////////////////////////////////////
	void negate();
	void reverse(); //reverse vertex order

	///////////////////////////////////////////////////////////////////////////
	void translate(const Vector2d& v);
	void rotate(double radius);

	///////////////////////////////////////////////////////////////////////////
	// Access
	ply_vertex * getHead() const { return head; }
	POLYTYPE getType() const { return type; }
	void set(POLYTYPE t,ply_vertex * h){
		type=t; head=h;
		if(h!=NULL){ tail=h->getPre(); }
		else{ tail=NULL; }
	}
	int getSize() {
		if(all.empty()) build_all();
		return all.size();
	}

	ply_vertex * operator[](unsigned int id){
		if(all.empty()) build_all();
		return all[id];
	}

	///////////////////////////////////////////////////////////////////////////
	// additional functions
	const Point2d& getCenter();

	///////////////////////////////////////////////////////////////////////////
	//compute the Radius of the poly chain
	float getRadius();

	//area
	float getArea();

	//check if convex
	bool is_convex() const;

	//delete a vertex
	void delete_vertex(ply_vertex * p);

	///////////////////////////////////////////////////////////////////////////
	// Operator
	//check if give poly line is the same as this
	bool operator==( const c_ply& other ){ return other.head==head; }
	friend istream& operator>>( istream&, c_ply& );
	friend ostream& operator<<( ostream&, c_ply& );

protected:

	///////////////////////////////////////////////////////////////////////////
	void doInit(); /*return # of vertice in this poly*/

	//build elements in vector<ply_vertex*> all
	void build_all();

private:

	ply_vertex * head; //the head of vertex list
	ply_vertex * tail; //end of the vertex list

	vector<ply_vertex*> all; //all vertices

	//additional info
	Point2d center;
	float radius;
	float area;

	//In, out or unknown.
	POLYTYPE type;
};


//a c_plylist is a list of c_ply
class c_plylist : public list<c_ply>
{
	friend ostream& operator<<( ostream&, c_plylist& );
	friend istream& operator>>( istream&, c_plylist& );

public:

	c_plylist()
	{
		box[0]=box[1]=box[2]=box[3]=0;
		is_buildboxandcenter_called=false;
	}

	void negate();
	void translate(const Vector2d& v);
	void rotate(double r);

	//access
	void buildBoxAndCenter();
	const double * getBBox() const { assert(is_buildboxandcenter_called); return box; }
	const Point2d& getCenter() const { assert(is_buildboxandcenter_called); return center; }

protected:

	Point2d center;
	double box[4];

private:

	bool is_buildboxandcenter_called;
};

//
// a c_polygon is a restricted kind of c_plylist
// this defines a simple polygon so that
// the first element much be a POUT c_ply and
// the rest ply lines are a list of holes
//
class c_polygon : public c_plylist
{
public:

	c_polygon() { area=0; m_isSplit = false; }

	bool valid(); //check if this is a valid polygon

	//copy from the given polygon
	void copy(const c_polygon& other);


	list<c_polygon> split();

	void reverse(); //reverse the vertex order (not the list order)

	//access the vertices of the polygon as an array
	int getSize()
	{
		if(all.empty()) build_all();
		return all.size();
	}

	ply_vertex * operator[](unsigned int id){
		if(all.empty()) build_all();
		return all[id];
	}

	double getArea();

	//destroy
	void destroy();

	//get number of vertices
	uint getSize() const;

	bool is_convex() const;

	bool getIsSplit() const { return m_isSplit;}
	void setIsSplit(bool value) { m_isSplit = value; }

	ply_vertex* getVertexByVID(uint vid);


private:

	void build_all();

	vector<ply_vertex*> all; //all vertices
	map<uint, uint> vid_index_map; // map for vid and index in all

	float area;

	bool m_isSplit;
};

#endif //_POLYGON_H_
