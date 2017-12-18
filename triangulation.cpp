#include "triangulation.h"
#include "intersection.h"
#include <stack>
#include <vector>
#include <queue>

//
//given a polygon, compute a list of triangles that partition the polygon
//
void Triangulator::triangulate(c_polygon& poly, vector<triangle>& triangles, int method )
{
	this->m_ploy.copy(poly);
	this->m_polys.push_back(this->m_ploy);

	if(method == 0){
		findMonotonicPolygons(poly);
		//Loop through monotonic polygons and create triangles
		numTri = 0;
		for(auto iter=this->m_polys.begin();iter!=this->m_polys.end();++iter){
			if(iter->getIsSplit()) continue;
			findTriangles(*iter, triangles);
		}
		cout << endl;
	}
	else if(method == 1 || method == 2){
		earClipping(this->m_ploy, triangles, method);
	}
	else if(method == 3){
		optimal(triangles);
	}
	else if(method == 4){
		bruteForce(triangles);
	}
}

// Used to test priority queue
template<typename T> void print_queue(T& q) {
	cout << "Priority Queue: [ ";
	while(!q.empty()) {
		cout << "( " << q.top()->getPos() << ") ";
		q.pop();
	}
	cout << "]\n";
}

// Used to test queue
template<typename T> void print_rqueue(T& q) {
	cout << "Queue: [ ";
	while(!q.empty()) {
		cout << "( " << q.front()->getPos() << ") ";
		q.pop();
	}
	cout << "]\n";
}

// Used to test m_bst (the Binary Search Tree)
void Visit(BTreeNode<ply_edge*, double> *u){
	cout << "{ [( " << u->data()->getSource()->getPos() << ") , ( " << u->data()->getTarget()->getPos() << ")] : ( "<< u->data()->getHelper()->getPos() << ") } ; ";
}

bool isLeftTurn(ply_vertex* a, ply_vertex* b, ply_vertex* c)
{
	Vector2d v = c->getPos()-b->getPos();
	Vector2d u = b->getPos()-a->getPos();
	float z=u[0]*v[1]-u[1]*v[0];
	if(z<=0) return true;
	return false;
}

float triangleArea(ply_vertex* a, ply_vertex* b, ply_vertex* c){
	const Point2d& p1 = a->getPos();
	const Point2d& p2 = b->getPos();
	const Point2d& p3 = c->getPos(); 
	return TriangleArea(p1.get(),p2.get(),p3.get());
}

bool isPointInsideTriangle(ply_vertex* p, ply_vertex* a, ply_vertex* b, ply_vertex* c){
	float A = triangleArea(a,b,c);
	float A1 = triangleArea(p,b,c);
	float A2 = triangleArea(p,a,c);
	float A3 = triangleArea(p,a,b);
	//cout << "\t\t\t\t\t\t\t" << A << " == " << A1+A2+A3 << " ? \n";
	//A = roundf(A * (10^6) ) / (10^6);
	float total = A1+A2+A3;
	//total = roundf(total * (10^6) ) / (10^6);
	return (A == total);
}

bool isTriangleInsidePoly(c_ply& ply, ply_vertex* a, ply_vertex* b, ply_vertex* c){
	bool insidePoly = false;
	if(!isLeftTurn(a,b,c))
		return false;
	ply_vertex* ptr = ply.getHead();
	//cout << "\tisPointInsideTriangle: "<< ply.getChainSize() <<" points to check\n";
	int i = 0;
	do{
		if(ptr->getPos() == a->getPos()) { ptr = ptr->getNext(); continue; }
		if(ptr->getPos() == b->getPos()) { ptr = ptr->getNext(); continue; }
		if(ptr->getPos() == c->getPos()) { ptr = ptr->getNext(); continue; }	
		
		insidePoly = isPointInsideTriangle(ptr,a,b,c);
		//cout << "\t\t"<< i << " : ( " << ptr->getPos() << "): " << insidePoly << "\n";
		if(insidePoly == true) break;
		ptr = ptr->getNext();
		++i;
	} 
	while(ptr != ply.getHead());
	
	//cout << "isTriangleInsidePoly: " << !insidePoly << "\n";
	return !insidePoly;
}

bool isTriangleInsidePoly(c_polygon& poly, ply_vertex* a, ply_vertex* b, ply_vertex* c){
	bool insidePoly = false;
	if(!isLeftTurn(a,b,c))
		return false;
	for(auto & ply : poly) {
		ply_vertex* ptr = ply.getHead();
		//cout << "\tisPointInsideTriangle: "<< ply.getChainSize() <<" points to check\n";
		int i = 0;
		do{
			if(ptr->getPos() == a->getPos()) { ptr = ptr->getNext(); continue; }
			if(ptr->getPos() == b->getPos()) { ptr = ptr->getNext(); continue; }
			if(ptr->getPos() == c->getPos()) { ptr = ptr->getNext(); continue; }	
			
			insidePoly = isPointInsideTriangle(ptr,a,b,c);
			//cout << "\t\t"<< i << " : ( " << ptr->getPos() << "): " << insidePoly << "\n";
			if(insidePoly == true) break;
			ptr = ptr->getNext();
			++i;
		} 
		while(ptr != ply.getHead());
	}
	//cout << "isTriangleInsidePoly: " << !insidePoly << "\n";
	return !insidePoly;
}

void Triangulator::earClipping(c_polygon& poly, vector<triangle>& triangles, int method){
	
	for(auto & ply : poly) {
					
		float total = ply.getChainSize();
		float i = ply.getChainSize();
		ply_vertex* ptr = ply.getHead();
		ply_vertex* ptr_copy = ply.getHead();
		do{
			//cout << "PRE( " << ptr->getPre()->getPos() << "), PTR( "<< ptr->getPos() << "), NEXT( " << ptr->getNext()->getPos() << ") \n";
			if(isTriangleInsidePoly(poly, ptr->getNext(), ptr, ptr->getPre()))
			{
				addTriangle(triangles,ptr->getNext()->getVID(),ptr->getVID(),ptr->getPre()->getVID());
				ptr->getPre()->setNext(ptr->getNext());
				ptr->getNext()->setPre(ptr->getPre());
				if(ply.getHead() == ptr)
					ply.setHead(ptr->getPre());
				
				if(method == 1)
					ptr=ptr->getNext();
				else
					ptr=ptr->getNext()->getNext();
			}
			else
				ptr=ptr->getNext();
				
			if(i != ply.getChainSize()){
				ptr_copy = ptr->getPre();
				i = ply.getChainSize();
				
				//////////////////////////////////////////////////////////////////
				// PROGRESS BAR
				//////////////////////////////////////////////////////////////////
				
				float prog = (total-i)/(total-2);
				int barWidth = 70; 
				cout << "[";
				int pos = barWidth * prog;
				for (int i = 0; i < barWidth; ++i) {
					if (i < pos) cout << "=";
					else if (i == pos) cout << ">";
					else cout << " ";
				}
				cout << "] " << int(prog * 100.0) << "% - "<< (total-i) << "/" << total-2 << "\r"<< flush;
				cout.flush();
				
				//////////////////////////////////////////////////////////////////
			}
			else{
				if(ptr->getPos() == ptr_copy->getPos())
					break;
			}
			//cout << i << endl;	
		}
		while(ply.getChainSize() >= 3 );
		cout << endl;
	}
}

double triangleCost(ply_vertex* a, ply_vertex* b, ply_vertex* c){
	return ply_edge(a,b).distance() 
		 + ply_edge(b,c).distance() 
		 + ply_edge(c,a).distance();
}

#define MAX 1000000.0

void Triangulator::findMinTriangle(c_polygon& poly, vector<triangle>& triangles){
	for(auto & ply : poly) {
		int n = ply.getChainSize();
		if(n == 3)
			return;
		
		ply_vertex* points[n];
		ply_vertex* ptr = ply.getHead();
		int i = 0;
		do{
			points[i] = ptr;
			ptr=ptr->getNext();
			++i;
		}
		while(ptr!=ply.getHead());

		double cTable[n][n];
		ply_vertex* tTable[n][n][3];
		for (int gap = 0; gap < n; gap++){
			for (int i = 0, j = gap; j < n; i++, j++){
				if (j < i+2)
					cTable[i][j] = 0.0;
				else{
					cTable[i][j] = MAX;
					for (int k = i+1; k < j; k++){
						double val = cTable[i][k] + cTable[k][j] + triangleCost(points[i],points[j],points[k]);
						if (cTable[i][j] > val && isTriangleInsidePoly(ply,points[i],points[j],points[k])){
							cTable[i][j] = val;
							tTable[i][j][0] = points[i];
							tTable[i][j][1] = points[j];
							tTable[i][j][2] = points[k];
						}
					}
				}
			}
		}
		ply_vertex* v1 = tTable[0][n-1][0];
		ply_vertex* v2 = tTable[0][n-1][1];
		ply_vertex* v3 = tTable[0][n-1][2];
		//cout << "Dia: {( " << v1->getPos() << "), ( "<< v2->getPos() << ")} \n";
		//cout << "Dia: {( " << v2->getPos() << "), ( "<< v3->getPos() << ")} \n";
		//cout << "Dia: {( " << v3->getPos() << "), ( "<< v1->getPos() << ")} \n"; 
		addDiagonal(v1, v2); 
		addDiagonal(v2, v3);		
		addDiagonal(v3, v1);
		break;
	} 
}

void Triangulator::optimal(vector<triangle>& triangles){
	bool change = true;
	int j = 0;
	int num_tri = 0;
	int total_tri = this->m_ploy.getSize()-2;
	do{
		//cout << "Iteration " << j << endl;
		int i = 0;
		for(auto iter=this->m_polys.begin();iter!=this->m_polys.end();++iter){
			int oldNum = distance(m_polys.begin(), m_polys.end());			
						
			//////////////////////////////////////////////////////////////////
			// PROGRESS BAR
			//////////////////////////////////////////////////////////////////
			
			float prog = (num_tri*1.0)/(total_tri);
			int barWidth = 70; 
			cout << "[";
			int pos = barWidth * prog;
			for (int i = 0; i < barWidth; ++i) {
				if (i < pos) cout << "=";
				else if (i == pos) cout << ">";
				else cout << " ";
			}
			cout << "] " << int(prog * 100.0) << "% - " << num_tri << "/" << total_tri << "\r"<< flush;
			cout.flush();
			
			//////////////////////////////////////////////////////////////////
			
			//cout << "\t" << i << " : Number of polys: " << oldNum << endl;
			++i;
			if(iter->getIsSplit()) continue;
			findMinTriangle(*iter,triangles);
			int newNum = distance(m_polys.begin(), m_polys.end());
			if(oldNum < newNum){
				++num_tri;
				//cout << "\t[" << oldNum << "," << newNum << "]" << endl;
				change = true;
				break;
			}
			else if(i == oldNum-1){
				change = false;	
			}
		}
		++j;
	} while(change);
	
	//////////////////////////////////////////////////////////////////
	// PROGRESS BAR
	//////////////////////////////////////////////////////////////////
	
	int barWidth = 70; 
	cout << "[";
	int pos = barWidth * 1;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) cout << "=";
		else if (i == pos) cout << ">";
		else cout << " ";
	}
	cout << "] " << int(1 * 100.0) << "% - " << total_tri << "/" << total_tri << "\r"<< flush;
	cout.flush();
	
	//////////////////////////////////////////////////////////////////
	
	cout << endl;
	
	for(auto iter=this->m_polys.begin();iter!=this->m_polys.end();++iter){
		if(iter->getIsSplit()) continue;	
		for(auto & ply : *iter) {
			if(ply.getChainSize() == 3){
				ply_vertex* v1 = ply.getHead();
				ply_vertex* v2 = v1->getNext();
				ply_vertex* v3 = v2->getNext();
				addTriangle(triangles,v1->getVID(),v2->getVID(),v3->getVID());		
			}
			else{
				cout << "Something went wrong!!!!!";
			}
		}
	}
}

int myrandom (int i) { return std::rand()%i;}

void Triangulator::bruteForce(vector<triangle>& triangles){
	bool change = true;
	int j = 0;
	int num_tri = 0;
	int total_tri = this->m_ploy.getSize()-2;
	do{
		//cout << "Iteration " << j << endl;
		int i = 0;
		for(auto iter=this->m_polys.begin();iter!=this->m_polys.end();++iter){
			int oldNum = distance(m_polys.begin(), m_polys.end());			
						
			//////////////////////////////////////////////////////////////////
			// PROGRESS BAR
			//////////////////////////////////////////////////////////////////
			
			float prog = (num_tri*1.0)/(total_tri);
			int barWidth = 70; 
			cout << "[";
			int pos = barWidth * prog;
			for (int i = 0; i < barWidth; ++i) {
				if (i < pos) cout << "=";
				else if (i == pos) cout << ">";
				else cout << " ";
			}
			cout << "] " << int(prog * 100.0) << "% - " << num_tri << "/" << total_tri << "\r"<< flush;
			cout.flush();
			
			//////////////////////////////////////////////////////////////////
			
			//cout << "\t" << i << " : Number of polys: " << oldNum << endl;
			++i;
			if(iter->getIsSplit()) continue;
			bruteForce(*iter,triangles);
			int newNum = distance(m_polys.begin(), m_polys.end());
			if(oldNum < newNum){
				++num_tri;
				//cout << "\t[" << oldNum << "," << newNum << "]" << endl;
				change = true;
				break;
			}
			else if(i == oldNum-1){
				change = false;	
			}
		}
		++j;
	} while(change);
	
	//////////////////////////////////////////////////////////////////
	// PROGRESS BAR
	//////////////////////////////////////////////////////////////////
	
	int barWidth = 70; 
	cout << "[";
	int pos = barWidth * 1;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) cout << "=";
		else if (i == pos) cout << ">";
		else cout << " ";
	}
	cout << "] " << int(1 * 100.0) << "% - " << total_tri << "/" << total_tri << "\r"<< flush;
	cout.flush();
	
	//////////////////////////////////////////////////////////////////
	
	cout << endl;
	
	/*for(auto iter=this->m_polys.begin();iter!=this->m_polys.end();++iter){
		if(iter->getIsSplit()) continue;	
		for(auto & ply : *iter) {
			if(ply.getChainSize() == 3){
				ply_vertex* v1 = ply.getHead();
				ply_vertex* v2 = v1->getNext();
				ply_vertex* v3 = v2->getNext();
				addTriangle(triangles,v1->getVID(),v2->getVID(),v3->getVID());		
			}
			else{
				cout << "Something went wrong!!!!!";
			}
		}
	}*/
}

void Triangulator::bruteForce(c_polygon& poly, vector<triangle>& triangles){
	
	deque<ply_vertex*> m_dqueue;
	
	for(auto & ply : poly) {
		ply_vertex* ptr = ply.getHead();
		do{
			m_dqueue.push_back(ptr);
			ptr=ptr->getNext();
		}
		while(ptr!=ply.getHead());
	}
	
	srand(time(0));
	random_shuffle(m_dqueue.begin(),m_dqueue.end(), myrandom);
	queue<ply_vertex*> m_rqueue(m_dqueue);
	
	//print_rqueue(m_rqueue);
	
	while (!m_rqueue.empty()) {
		ply_vertex* v1 = m_rqueue.front();
		m_rqueue.pop();
		
		queue<ply_vertex*> m_rqueue2(m_rqueue);
		while (!m_rqueue2.empty()) {
			ply_vertex* v2 = m_rqueue2.front();
			m_rqueue2.pop();
			
			queue<ply_vertex*> m_rqueue3(m_rqueue);
			while (!m_rqueue3.empty()) {
				//cout << m_rqueue3.size() << endl;
				ply_vertex* v3 = m_rqueue3.front();
				m_rqueue3.pop();
				
				if(isTriangleInsidePoly(poly, v1, v2, v3)){
					
					bool doAdd = true;
					for( auto & tri : triangles){
						if((tri.v[0] == v1->getVID() && tri.v[1] == v2->getVID() && tri.v[2] == v3->getVID()) 
						|| (tri.v[0] == v1->getVID() && tri.v[1] == v3->getVID() && tri.v[2] == v2->getVID())
						|| (tri.v[0] == v2->getVID() && tri.v[1] == v3->getVID() && tri.v[2] == v1->getVID())
						|| (tri.v[0] == v2->getVID() && tri.v[1] == v1->getVID() && tri.v[2] == v3->getVID()) 
						|| (tri.v[0] == v3->getVID() && tri.v[1] == v2->getVID() && tri.v[2] == v1->getVID())
						|| (tri.v[0] == v3->getVID() && tri.v[1] == v1->getVID() && tri.v[2] == v3->getVID()) ){
							
							doAdd = false;
						}
					}
					if((v1->getVID() == v2->getVID() == v3->getVID()) 
						|| (v1->getVID() == v2->getVID())
						|| (v2->getVID() == v3->getVID())
						|| (v1->getVID() == v3->getVID()) ){
							
							doAdd = false;
					}
					if(doAdd){
						cout << "( " << v1->getPos() << "), ( "<< v2->getPos() << "), ( " << v3->getPos() << ") \n";
						
						addDiagonal(v1, v2); 
						addDiagonal(v2, v3);		
						addDiagonal(v3, v1);
						
						addTriangle(triangles,v1->getVID(),v2->getVID(),v3->getVID());
						
						goto end_loop;
					}
				}
			}
		}
	}
	end_loop:
		cout<< "\n";
}

//
// given a polygon, split it into monotonic polygons
//
void Triangulator::findMonotonicPolygons(c_polygon& poly)
{
	// 1. Construct a priority queue Q (m_pqueue) on the vertices of P (poly), 
	// using their y-coordinates as priority. If two points have the same 
	// y-coordinate, the one with smaller x-coordinate has higher priority.	
	for(auto & ply : poly) {
		if(ply.getType() == c_ply::POUT) {
			ply_vertex* ptr = ply.getHead();
			do{
				this->m_pqueue.push(ptr);
				ptr=ptr->getNext();
			}
			while(ptr!=ply.getHead());
		}
	}
	//print_queue(this->m_pqueue);
	//print_queue(this->h_pqueue);
	
	// 2. Initialize an empty binary search tree T.
	// its already empty
		
	// 3. While Q is not empty
	int i = 0;	
	while (!this->m_pqueue.empty()) {
		// 4. do Remove the vertex vi with the highest priority from Q
		ply_vertex* vertex = this->m_pqueue.top();
		this->m_pqueue.pop();
		// 5. Call the appropriate procedure to handle the vertex, depending on its type.
		//cout << i << ": " << vertex->getVID() << "\n";
		if (vertex->getType() == ply_vertex::START)
			handleStartVertex(vertex);
		else if (vertex->getType() == ply_vertex::END)
			handleEndVertex(vertex);	
		else if (vertex->getType() == ply_vertex::SPLIT)
			handleSplitVertex(vertex);
		else if (vertex->getType() == ply_vertex::MERGE)
			handleMergeVertex(vertex);
		else if (vertex->getType() == ply_vertex::REGULAR_DOWN)
			handleRegularDownVertex(vertex);
		else if (vertex->getType() == ply_vertex::REGULAR_UP) 
			handleRegularUpVertex(vertex);
		else {
			cerr << "This vertex is a Unknown Type";
		}
		++i;
	}
	/*for(int i=0; i<m_edges.size(); ++i){
		cout << "\t" << i << ": ";
		if(m_edges[i] != 0)
			cout << m_edges[i]->keyValue();
		else
			cout << m_edges[i];
		cout << "\n";
 	}*/
}

//
// given a monotonic polygon, compute a list of triangles that partition the polygon
//
void Triangulator::findTriangles(c_polygon& poly, vector<triangle>& triangles)
{
	//cout << "\nFinding Triangles for Monotone Polygon:\n";
	// 1. Merge the vertices on the left chain and the vertices on the right
	// chain of P into one sequence, sorted on decreasing y-coordinate. If two 
	// vertices have the same y-coordinate, then the leftmost one comes first. 
	// Let u1,...,un denote the sorted sequence.
	for(auto & ply : poly) {
		ply_vertex* ptr = ply.getHead();
		do{
			this->m_pqueue.push(ptr);
			ptr=ptr->getNext();
		}
		while(ptr!=ply.getHead());
	}
	//print_queue(this->m_pqueue);
	
	// 2. Initialize an empty stack S, and push u1 and u2 onto it.
	stack<ply_vertex*> m_stack;
	
	m_stack.push(this->m_pqueue.top()); // Start Vertex
	this->m_pqueue.pop();
	
	m_stack.push(this->m_pqueue.top());
	this->m_pqueue.pop();
	
	//int numTri = 0;
	int totalTri = this->m_ploy.getSize()-2;
	
	while(m_pqueue.size() > 1) {
		ply_vertex* e = this->m_pqueue.top();
		this->m_pqueue.pop();
		
		//if e and the vertex on top of S are on different chains
		if(((m_stack.top()->getType() == ply_vertex::REGULAR_UP) && (e->getType() == ply_vertex::REGULAR_DOWN))
			|| ((m_stack.top()->getType() == ply_vertex::REGULAR_DOWN) && (e->getType() == ply_vertex::REGULAR_UP))) 
		{
			//cout << "e:( " << e->getPos() << ") and stack.top():( " << m_stack.top()->getPos() << ") on different chains\n";	
			ply_vertex* x = m_stack.top();
			ply_vertex* a = x;
			m_stack.pop();
			while(m_stack.size() >= 1){
				ply_vertex* b = m_stack.top(); 
				m_stack.pop();
				//cout << "\te:( " << e->getPos() << "), a:( " << a->getPos() << "), b:( " << b->getPos() << ")\n";
				//cout << "\t\tTriangle added\n";
				addTriangle(triangles,e->getVID(),a->getVID(),b->getVID());
				++numTri;
				a = b;
			}
			m_stack.push(x);
			m_stack.push(e);
		}
		//else if e and the vertex on top of S are on the same chain
		else if(((m_stack.top()->getType() == ply_vertex::REGULAR_UP) && (e->getType() == ply_vertex::REGULAR_UP))
			|| ((m_stack.top()->getType() == ply_vertex::REGULAR_DOWN) && (e->getType() == ply_vertex::REGULAR_DOWN))) 
		{
			//cout << "e:( " << e->getPos() << ") and stack.top():( " << m_stack.top()->getPos() << ") on same chain\n";	
			while(m_stack.size() >= 2){
				ply_vertex* a = m_stack.top(); 
				m_stack.pop();
				ply_vertex* b = m_stack.top(); 
				m_stack.pop();
				//cout << "\te:( " << e->getPos() << "), a:( " << a->getPos() << "), b:( " << b->getPos() << ")\n";
				// if eab is a left turn and e is Regular Down OR if bae is a left turn and e is Regular Up
				if((isLeftTurn(e,a,b) && (e->getType() == ply_vertex::REGULAR_DOWN))
					|| (isLeftTurn(b,a,e) && (e->getType() == ply_vertex::REGULAR_UP)))
				{	
					//cout << "\t\tTriangle added\n";
					addTriangle(triangles,e->getVID(),a->getVID(),b->getVID());
					++numTri;
					m_stack.push(b);
				}
				else{
					//cout << "\t\tTriangle not added\n";
					m_stack.push(b);
					m_stack.push(a);
					break;
				}
			}
			m_stack.push(e);
		}
		else
			cerr << "One of the vertices is a Merge, Split, or Unknown Type\n";
			
		//////////////////////////////////////////////////////////////////
		// PROGRESS BAR
		//////////////////////////////////////////////////////////////////
		
		float prog = (numTri*1.0)/(totalTri);
		int barWidth = 70; 
		cout << "[";
		int pos = barWidth * prog;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) cout << "=";
			else if (i == pos) cout << ">";
			else cout << " ";
		}
		cout << "] " << int(prog * 100.0) << "% - "<< numTri << "/" << totalTri << "\r"<< flush;
		cout.flush();
		
		//////////////////////////////////////////////////////////////////
	}
	
	ply_vertex* e = this->m_pqueue.top();
	this->m_pqueue.pop();
	
	ply_vertex* a = m_stack.top();
	m_stack.pop();
	//cout << "e:( " << e->getPos() << ") is End vertex with a:( " << a->getPos() << ")\n";
	while(m_stack.size() >= 1){
		ply_vertex* b = m_stack.top(); 
		m_stack.pop();
		//cout << "\te:( " << e->getPos() << "), a:( " << a->getPos() << "), b:( " << b->getPos() << ")\n";
		//cout << "\t\tTriangle added\n";
		addTriangle(triangles,e->getVID(),a->getVID(),b->getVID());
		++numTri;
		a = b; 
		
		//////////////////////////////////////////////////////////////////
		// PROGRESS BAR
		//////////////////////////////////////////////////////////////////
		
		float prog = (numTri*1.0)/(totalTri);
		int barWidth = 70; 
		cout << "[";
		int pos = barWidth * prog;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) cout << "=";
			else if (i == pos) cout << ">";
			else cout << " ";
		}
		cout << "] " << int(prog * 100.0) << "% - "<< numTri << "/" << totalTri << "\r"<< flush;
		cout.flush();
		
		//////////////////////////////////////////////////////////////////
	}
}

// handle start vertex
void Triangulator::handleStartVertex(ply_vertex* vertex)
{
	//cout << "Start: " << vertex->getPos() << "\n";
	insertEI(vertex);
}

// handle end vertex
void Triangulator::handleEndVertex(ply_vertex* vertex)
{
	//cout << "End: " << vertex->getPos() << "\n";
	deletePreEI(vertex);
}

// handle merge vertex
void Triangulator::handleMergeVertex(ply_vertex* vertex)
{
	//cout << "Merge: " << vertex->getPos() << "\n";
	deletePreEI(vertex);
	updateEJ(vertex);
}

// handle split vertex
void Triangulator::handleSplitVertex(ply_vertex* vertex)
{
	//cout << "Split: " << vertex->getPos() << "\n";
	updateEJ(vertex);
	insertEI(vertex);
}

// handle regular up vertex, P lies on left of the vertex
void Triangulator::handleRegularUpVertex(ply_vertex* vertex)
{
	//cout << "Regular Up: " << vertex->getPos() << "\n";
	updateEJ(vertex);
}

// handle regular down vertex, P lies on right of the vertex
void Triangulator::handleRegularDownVertex(ply_vertex* vertex)
{
	//cout << "Regular Down: " << vertex->getPos() << "\n";
	deletePreEI(vertex);
	insertEI(vertex);
}

// ADDED METHOD
// Steps to insert ei
void Triangulator::insertEI(ply_vertex* vertex)
{
	//cout << "\tInsert: " << vertex->getPos() << "\n";
	// Insert ei in T and set helper(ei) to vi
	insertEdgeIntoBST(vertex);
}

// ADDED METHOD
// Steps to delete ei-1
void Triangulator::deletePreEI(ply_vertex* vertex)
{
	//cout << "\tDelete: " << vertex->getPos() << "\n";
	ply_edge* edge_pre_i = this->m_edges[vertex->getPre()->getVID()]; // ei-1
	ply_vertex* helper_e_pre_i = edge_pre_i->getHelper(); // helper(ei-1)
	// if helper(ei-1) is a merge vertex
	if(helper_e_pre_i->getType() == ply_vertex::MERGE){
		// then Insert the diagonal connecting vi to helper(ei-1) in D.
		//cout << "\t\tDiagonal: ( " << vertex->getPos() << ") ( " << helper_e_pre_i->getPos() << ")\n";
		addDiagonal(vertex, helper_e_pre_i);
	}
	// Delete ei-1 from T.
	deleteEdgeFromBST(edge_pre_i);	
}

// ADDED METHOD
// Steps to update ej
void Triangulator::updateEJ(ply_vertex* vertex)
{
	//cout << "\tUpdate: " << vertex->getPos() << "\n";
	BTreeNode<ply_edge*,double>* ej_node = new BTreeNode<ply_edge*,double>();
	// Search in T to find the edge ej directly left of vi
	this->m_bst.FindMaxSmallerThan(vertex->getPos()[0], ej_node);
	ply_edge* edge_j = ej_node->data();
	ply_vertex* helper_ej = edge_j->getHelper();
	
	// if helper(ej) is a merge vertex OR vi is a split vertex
	if(helper_ej->getType() == ply_vertex::MERGE || vertex->getType() == ply_vertex::SPLIT){
		// then Insert the diagonal connecting vi to helper(ej) in D.
		//cout << "\t\tDiagonal: ( " << vertex->getPos() << ") ( " << helper_ej->getPos() << ")\n";
		addDiagonal(vertex, helper_ej);
	}
	// helper(ej) <- vi
	edge_j->setHelper(vertex);
	/*cout << "\t\tBST: ";
	this->m_bst.PostOrder(Visit);
	cout << "\n";*/
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
	if(source->getNext()->getVID() == target->getVID() || target->getNext()->getVID() == source->getVID())
		return;
		
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
