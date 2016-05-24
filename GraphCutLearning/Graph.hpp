#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_

#include <cassert>

class Graph
{
public:
	typedef int tagT;
	typedef double weightT;
	typedef unsigned int arraySizeT;
	struct Node;
	struct Nbhd;

	struct Node
	{
		Nbhd * nb = nullptr; // array of neighbors
		tagT tag; // name / tag of node. WARNING: do NOT use 0, -1, -2
		arraySizeT place; // place in node array
	};

	struct Nbhd final // vecNeighbors
	{
		Node * n /**= nullptr*/; // ptr to neighbor node
		arraySizeT link; // number of link weight in sequence
		bool last /**= false*/;
		/// + 24 align
	};

	Graph() = delete;
	Graph(const arraySizeT nodenum, const arraySizeT linknum) : node(new Node[nodes = nodenum]), linkweight(new weightT[links = linknum]) {}
	//Graph(const Graph & gr);
	Graph(Graph && gr);// noexcept;
	~Graph();

	//Graph & operator = (const Graph & gr);
	Graph & operator = (Graph && gr);// noexcept;

	Node & operator[] (const arraySizeT i) { return node[i]; }
	weightT & operator() (const arraySizeT i) { return linkweight[i]; }
	const Node & operator[] (const arraySizeT i) const { return node[i]; }
	const weightT & operator() (const arraySizeT i) const { return linkweight[i]; }
	const arraySizeT nodenum() const { return nodes; }
	const arraySizeT linknum() const { return links; }

private:
	Node * node;
	weightT * linkweight;	// weights of all links
	arraySizeT nodes, links;
	/// + 32 align
};

/*inline Graph::Graph(const Graph & gr): node(new Node[nodes=gr.nodes]), linkweight(new weightT[links=gr.links])
{
for (arraySizeT i = 0; i < nodes; ++i)
{
node[i].nb = new Nbhd[nbmax];
node[i].tag = gr.node[i].tag;
for (arraySizeT j = 0; j < nbmax; ++j)
if ( gr.node[i].nb[j].n )
{
node[i].nb[j].n = gr.node[i].nb[j].n;
node[i].nb[j].link = gr.node[i].nb[j].link;
}
else
{
node[i].nb[j].n = nullptr;
break;
}
}
}*/

inline Graph::Graph(Graph && gr) /*noexcept*/ : node(gr.node), linkweight(gr.linkweight), nodes(gr.nodes), links(gr.links)
{
	gr.node = nullptr;
	gr.linkweight = nullptr;
}

inline Graph::~Graph()
{
	if (node)
		for (arraySizeT i = 0; i < nodes; ++i)
			delete[] node[i].nb;
	delete[] node;
	delete[] linkweight;
}

inline Graph & Graph::operator = (Graph && gr) //noexcept
{
	assert(this != &gr);
	if (node)
		for (arraySizeT i = 0; i < nodes; ++i)
			delete[] node[i].nb;
	delete[] node;
	delete[] linkweight;
	node = gr.node;
	linkweight = gr.linkweight;
	nodes = gr.nodes;
	links = gr.links;
	gr.node = nullptr;
	gr.linkweight = nullptr;
	gr.nodes = 0;
	return *this;
}

/*inline Graph & Graph::operator = (const Graph & gr)
{
if (this == &gr) return *this;
for (arraySizeT i = 0; i < nodes; ++i)
delete[] node[i].nb;
delete[] node;
delete[] linkweight;
node = new Node[nodes=gr.nodes];
linkweight = new weightT[links=gr.links];
nbmax = gr.nbmax;
for (arraySizeT i = 0; i < nodes; ++i)
{
node[i].nb = new Nbhd[nbmax];
node[i].tag = gr.node[i].tag;
for (arraySizeT j = 0; j < nbmax; ++j)
if ( gr.node[i].nb[j].n )
{
node[i].nb[j].n = gr.node[i].nb[j].n;
node[i].nb[j].link = gr.node[i].nb[j].link;
}
else
{
node[i].nb[j].n = nullptr;
break;
}
}
return *this;
}*/

#endif // _GRAPH_HPP_
