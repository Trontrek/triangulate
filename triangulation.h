#pragma once

#include <queue>
#include <vector>
#include <map>
#include <set>

#include "polygon.h"
#include "splay.h"
using namespace std;

//
// New
//
template <class T, class KeyType>  class SplayTree;
typedef SplayTree<ply_edge*, double> EdgeBST;
typedef priority_queue<ply_vertex*, vector<ply_vertex*>, CompareVertex> PQueue;
typedef map<int, ply_edge*> EdgeMap;

//
//a triangle
//
struct triangle
{
	triangle(){ v[0]=v[1]=v[2]=UINT_MAX; }
	triangle(uint a, uint b, uint c){v[0]=a;v[1]=b;v[2]=c;}
	uint v[3]; // id to the vertices
};



class Triangulator
{

public:

	Triangulator()
	{
		//nothing yet
	}

	//
	//given a polygon, compute a list of triangles that partition the polygon
	//
	void triangulate(c_polygon& poly, vector<triangle>& triangles );

	vector<ply_edge>* getDiagonals() { return &m_diagonals; }

protected:

	//
	//given a polygon, split it into monotonic polygons
	//
	void findMonotonicPolygons(c_polygon& poly);

	//
	// given a monotonic polygon, compute a list of triangles that partition the polygon
	//
	void findTriangles(c_polygon& poly, vector<triangle>& triangles);

	// handle start vertex event
	void handleStartVertex(ply_vertex* vertex);

	// handle end vertex event
	void handleEndVertex(ply_vertex* vertex);

	// handle merge vertex event
	void handleMergeVertex(ply_vertex* vertex);

	// handle split vertex event
	void handleSplitVertex(ply_vertex* vertex);

	// handle regular down vertex event
	void handleRegularDownVertex(ply_vertex* vertex);

	// handle regular up vertex event
	void handleRegularUpVertex(ply_vertex* vertex);
	
private:

	// add triangle into triangles with vids of three vertices
	void addTriangle(vector<triangle>& triangles, int vid1, int vid2, int vid3);

	// add an diagonal from source to target;
	void addDiagonal(ply_vertex* source, ply_vertex* target);

	// insert an edge into BST
	void insertEdgeIntoBST(ply_vertex* vertex);

	// delete an edge from BST
	void deleteEdgeFromBST(ply_edge* edge);

	//
	// find the split polygon which contains the edge from source to target
	//
	c_polygon* getPolygonByDiagonal(uint source_vid, uint target_vid);

	c_polygon findSubPolygonByDiagonal(c_polygon& orginal, uint source_vid, uint target_vid, bool CCW=true);

	PQueue m_pqueue;
	EdgeBST m_bst;
	EdgeMap m_edges;
	vector<ply_edge> m_diagonals;
	c_polygon m_ploy;
	vector<c_polygon> m_polys;

};
