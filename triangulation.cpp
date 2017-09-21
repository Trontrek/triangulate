#include "triangulation.h"
#include "intersection.h"

//
//given a polygon, compute a list of triangles that partition the polygon
//
void Triangulator::triangulate(c_polygon& poly, vector<triangle>& triangles )
{
	this->m_ploy.copy(poly);
	this->m_polys.push_back(this->m_ploy);

	findMonotonicPolygons(poly);

  //Loop through monotonic polygons and create triangles
	for(auto iter=this->m_polys.begin();iter!=this->m_polys.end();++iter)
	{
		if(iter->getIsSplit()) continue;
		findTriangles(*iter, triangles);
	}
}

//
// given a polygon, split it into monotonic polygons
//
void Triangulator::findMonotonicPolygons(c_polygon& poly)
{
	//Your implementation here
}

//
// given a monotonic polygon, compute a list of triangles that partition the polygon
//
void Triangulator::findTriangles(c_polygon& poly, vector<triangle>& triangles)
{
	//Your implementation here
}

// handle start vertex
void Triangulator::handleStartVertex(ply_vertex* vertex)
{
	//Your implementation here
}

// handle end vertex
void Triangulator::handleEndVertex(ply_vertex* vertex)
{
	//Your implementation here
}

// handle merge vertex
void Triangulator::handleMergeVertex(ply_vertex* vertex)
{
	//Your implementation here
}

// handle split vertex
void Triangulator::handleSplitVertex(ply_vertex* vertex)
{
	//Your implementation here
}


// handle regular up vertex, P lies on left of the vertex
void Triangulator::handleRegularUpVertex(ply_vertex* vertex)
{
	//Your implementation here
}

// handle regular down vertex, P lies on right of the vertex
void Triangulator::handleRegularDownVertex(ply_vertex* vertex)
{
	//Your implementation here
}



//-----------------------------------------------------------------------------
//
//
// Private methods below, do not change
//
//
//-----------------------------------------------------------------------------



void Triangulator::addTriangle(vector<triangle>& triangles, int vid1, int vid2, int vid3)
{
	triangle tri(vid1, vid2, vid3);

	triangles.push_back(tri);
}

void Triangulator::addDiagonal(ply_vertex* source, ply_vertex* target)
{
	ply_edge diagonal(source, target);
	this->m_diagonals.push_back(diagonal);

	c_polygon* poly = this->getPolygonByDiagonal(source->getVID(), target->getVID());
	c_polygon ccw = this->findSubPolygonByDiagonal(*poly, source->getVID(), target->getVID(), true);
	c_polygon cw = this->findSubPolygonByDiagonal(*poly, source->getVID(), target->getVID(), false);

	this->m_polys.push_back(cw);
	this->m_polys.push_back(ccw);
}

void Triangulator::insertEdgeIntoBST(ply_vertex* vertex)
{
	double yPos = vertex->getPos()[1];
	ply_edge* edge = new ply_edge(vertex, (vertex->getNext()));
	edge->setHelper(vertex);
	edge->setKeyValue(yPos);

	this->m_edges[vertex->getVID()] = edge;

	this->m_bst.Insert(edge);
}

void Triangulator::deleteEdgeFromBST(ply_edge* edge)
{
	this->m_bst.Delete(edge->keyValue());
}


c_polygon* Triangulator::getPolygonByDiagonal(uint source_vid, uint target_vid)
{
	typedef vector<c_polygon>::iterator PIT;
	for(PIT it = m_polys.begin(); it!= m_polys.end(); ++it)
	{
		if(it->getIsSplit()) continue;

		int count = 0;

		for(int i=0;i<it->getSize();i++)
		{
			ply_vertex* vertex = (*it)[i];
			if(vertex->getVID() == source_vid || vertex->getVID() == target_vid)
				count++;
			if(count == 2) break;
		}

		if(count == 2)
		{
			c_polygon* poly = &(*it);

			poly->setIsSplit(true);

			return poly;
		}
	}

	cerr<<"! Error: Cannot found edge "<<source_vid<<" - "<<target_vid<<endl;

	assert(false);

	return NULL;
}

c_polygon Triangulator::findSubPolygonByDiagonal(c_polygon& poly, uint source_vid, uint target_vid, bool CCW)
{
	ply_vertex* sourceOrg = poly.getVertexByVID(source_vid);
	ply_vertex* targetOrg = poly.getVertexByVID(target_vid);
	ply_vertex* startVetex = CCW ? targetOrg : sourceOrg;
	ply_vertex* endVetex = CCW ? sourceOrg : targetOrg;
	ply_vertex* current = startVetex;

	c_ply sub_poly(c_ply::POUT);
	sub_poly.beginPoly();

	while(current->getVID() != endVetex->getVID())
	{
		sub_poly.addVertex(current->getPos()[0], current->getPos()[1], current->getVID());
		current = current->getNext();
	}

	sub_poly.addVertex(endVetex->getPos()[0], endVetex->getPos()[1], endVetex->getVID());
	sub_poly.endPoly(true);

	c_polygon new_poly;

	new_poly.push_back(sub_poly);

	return new_poly;
}
