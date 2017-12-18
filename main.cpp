//------------------------------------------------------------------------------
//  Copyright 2010-2017 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <unistd.h>
using namespace std;

#include "polygon.h"
#include "triangulation.h"
#include "simple_svg_1.0.0.h"

void draw_SVG_ply(svg::Document & doc, const c_polygon& polygon);
void draw_SVG_triangles(svg::Document & doc, c_polygon& poly, const vector<triangle>& triangles);

//
// Start of the main program
//
int main(int argc, char ** argv)
{
	if(argc<2){
		cerr<<"Usage: "<<argv[0]<<"[-m method] [-s scale] *.poly"<<endl;
		return 1;
	}

	float scale = 1;
	std::string poly_filename;
	int method = 0;

	for(int i=1;i<argc;i++){
		if(std::string(argv[i])=="-s"){
			scale=atof(argv[++i]);
			if(scale<0) scale=1;
		}
		else if(std::string(argv[i])=="-m"){
			++i;
			if(std::string(argv[i])=="monotone")
				method = 0;
			else if(std::string(argv[i])=="earclip_fan")
				method = 1;
			else if(std::string(argv[i])=="earclip_onion")
				method = 2;
			else if(std::string(argv[i])=="optimal")
				method = 3;
			else{
				cerr << "You have 4 choices of methods to choose from:\nmonotone, earclip_fan, earclip_onion, and optimal\n";
				return 1;
			}
		}
		else
			poly_filename=argv[i];
	}

	//open file
	ifstream fin(poly_filename.c_str());
	if(fin.good()==false)
	{
		cerr<<"! Error: Cannot open file: "<<poly_filename<<endl;
		return 1;
	}

	//create polygon
	c_polygon poly;
	fin>>poly;
	fin.close();

	//triangulate the polygon
	Triangulator trizor;
  	vector<triangle> triangles;	
  	trizor.triangulate(poly,triangles,method);

	//output information about the triangles
	cout<<"- There are "<<triangles.size()<<" triangles from a polygon with "
	    << poly.getSize()<<" vertices and "<<poly.size()
	    <<((poly.size()==1)?" boundary":" boundaries")<<endl;

	/*for( auto & tri : triangles){
		cout<<"\ttriangle: ";
		for(short j=0;j<3;j++) cout<<tri.v[j]<<" ";
		cout<<"\n";
	}
	cout<<flush;*/
	
	poly_filename.erase(0,12);
	poly_filename.erase(poly_filename.size()-5);
	
	string filename;
	if(method == 0)
		filename = "monotone_";
	else if(method == 1)
		filename = "earclip_fan_";
	else if(method == 2)
		filename = "earclip_onion_";
	else if(method == 3)
		filename = "optimal_";
	filename += poly_filename + ".svg";

	string SVGfilename(filename);

	poly.buildBoxAndCenter();
	const double * bbox = poly.getBBox();
	float width=(bbox[1]-bbox[0]);
	float height=(bbox[3]-bbox[2]);
	float delta=std::min(width,height)*0.05;

	svg::Dimensions dimensions( (width+delta*2)*scale, (height+delta*2)*scale);
	svg::Document doc(SVGfilename, svg::Layout(dimensions, svg::Layout::BottomLeft, scale, svg::Point(-bbox[0]+delta, -bbox[2]+delta) ));

	//draw polygon
	draw_SVG_ply(doc,poly);

	//draw triangles
	draw_SVG_triangles(doc,poly,triangles);

	//done
	doc.save();
	cout << "- Saved " << SVGfilename << endl;

	return 0;
}

void draw_SVG_ply(svg::Polygon & poly, const c_ply& ply)
{
	ply_vertex * ptr=ply.getHead();

	//loop through the verices
	do{
		const Point2d& pos=ptr->getPos();
		poly << svg::Point(pos[0],pos[1]);
		ptr=ptr->getNext();
	}
	while(ptr!=ply.getHead());
	poly.endBoundary();
}

void draw_SVG_ply(svg::Document & doc, const c_polygon& polygon)
{
	const double * bbox = polygon.getBBox();
	float thickness=std::min(bbox[1]-bbox[0],bbox[3]-bbox[2])*0.005f;
	svg::Polygon poly_bd(svg::Fill(svg::Color::Silver), svg::Stroke(thickness, svg::Color::Black));

	for(auto & ply : polygon)
	{
		if(ply.getType()==c_ply::POUT)
		{
			//draw the external boundary
			draw_SVG_ply(poly_bd, ply);
		}
		else{//Hole
			//svg::Polygon hole_bd(svg::Stroke(0.25, svg::Color::Green));
			draw_SVG_ply(poly_bd, ply);
		}
	}//end for i

	doc << poly_bd;
}

//draw triangles
void draw_SVG_triangles(svg::Document & doc, c_polygon& poly, const vector<triangle>& triangles)
{
	const double * bbox = poly.getBBox();
	float thickness=std::min(bbox[1]-bbox[0],bbox[3]-bbox[2])*0.0025f;

	//int j = 0;
	for( auto & tri : triangles)
	{
		svg::Polygon svgpoly(svg::Fill(svg::Color::Sky), svg::Stroke(thickness, svg::Color::Blue));
		//svg::Polygon svgpoly(svg::Fill(svg::Color::Lime), svg::Stroke(thickness, svg::Color::Green));
		/*if(j == 1){
			--j; --j;
			svg::Polygon svgpoly2(svg::Fill(svg::Color::Sky), svg::Stroke(thickness, svg::Color::Blue));
			svgpoly = svgpoly2;
		}
		++j;*/
		
		for(short i=0;i<3; i++)
		{
			const Point2d& p=poly[tri.v[i]]->getPos();
			svgpoly << svg::Point(p[0],p[1]);
		}

		svgpoly.endBoundary();
		doc << svgpoly;
	}
}
