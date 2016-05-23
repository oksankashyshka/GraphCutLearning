#ifndef _GRAPHCUT_HPP_
#define _GRAPHCUT_HPP_

#include <list>

#include "Graph.hpp"

class GraphCut
{
public:
	GraphCut() = delete;
	GraphCut(const GraphCut &) = delete;
	GraphCut(const Graph & gr);
	~GraphCut() { delete[] node; delete[] rflow; delete[] marking; }

	void mincut();
	const Graph::tagT * const marks() const { return marking; }

	const Graph::weightT * const operator() () const { return rflow; }
	Graph::weightT & operator[] (const Graph::arraySizeT i) { return rflow[i]; }

private:
	struct Node
	{
		const Graph::Node * par = nullptr;
		Graph::weightT maxflow = -1;
		Graph::arraySizeT parlink = -1;
		Graph::tagT tree = 0;
		Graph::tagT origin = 0;
		bool active = false;
		/// + 24 align
	};

	const Graph * gr;
	Node * node;
	Graph::weightT * rflow;
	Graph::tagT * marking;
	std::list<const Graph::Node *> active;

	void pathproc(const Graph::Node & p, const Graph::Node & q, const Graph::weightT flow, const Graph::arraySizeT link);
	void killorigin(const Graph::Node & c);
	void restoreorigin(const Graph::Node & c);
	//void mark(const Graph::Node & c);
};

inline GraphCut::GraphCut(const Graph & gr) : gr(&gr), node(new Node[gr.nodenum()]), rflow(new Graph::weightT[gr.linknum()]), marking(new Graph::tagT[gr.nodenum()] {0})
{
	for (Graph::arraySizeT i = 0; i < gr.linknum(); ++i)
	{
		rflow[i] = gr(i);
	}
	//memcpy( rflow, &gr(0), sizeof(Graph::weightT) * gr.linknum() );
}

#endif // _GRAPHCUT_HPP_
