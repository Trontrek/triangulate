#pragma once

#include "polygon.h"

//
//a triangle
//
struct triangle
{
	triangle(){ v[0]=v[1]=v[2]=UINT_MAX; }
	triangle(uint a, uint b, uint c){v[0]=a;v[1]=b;v[2]=c;}
	uint v[3]; // id to the vertices
};


//
//given a polygon, compute a list of triangles that partition the polygon
//
void triangulate(c_polygon& poly, vector<triangle>& triangles );
