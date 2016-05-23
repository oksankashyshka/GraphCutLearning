#include <limits>
#include <queue>

#include "Graphcut.hpp"

#include "debug.hpp"

#define DEBUG(x)

using std::list;
using std::queue;

typedef Graph::tagT tagT;
typedef Graph::weightT weightT;
typedef Graph::arraySizeT arraySizeT;
typedef Graph::Nbhd Nbhd;

static const weightT eps = 0.0000000001;

static inline const weightT min(const weightT a, const weightT b)
{
	return ( a < b ) ? a : b;
}

void GraphCut::mincut()
{
	DEBUG(	ProfTimer t;
			t.start();)
	node[0].maxflow = std::numeric_limits<double>::max();
	node[0].origin = node[0].tree = marking[0] = (*gr)[0].tag;
	node[0].active = true;
	node[1].maxflow = std::numeric_limits<double>::max();
	node[1].origin = node[1].tree = marking[1] = (*gr)[1].tag;
	node[1].active = true;
	active.push_back(&(*gr)[0]);
	active.push_back(&(*gr)[1]);

	for ( Graph::arraySizeT i = 2; i < gr->nodenum(); ++i )
		marking[i] = 0;

	show(rflow, reinterpret_cast<Nod*>(node));

	while ( !active.empty() )
	{
		const Graph::Node & p = *active.front();
		Node & n = node[ p.place ];
		active.pop_front();
		n.active = false;
		if ( p.nb )
		{
			for ( const Graph::Nbhd * nb = p.nb; ; ++nb )
			{
				if ( rflow[ nb->link ] > eps )
				{
					Node & nbn = node[ nb->n->place ];
					if ( !nbn.tree )
					{
						nbn.par = &p;
						nbn.maxflow = min( n.maxflow, rflow[nb->link] );
						nbn.parlink = nb->link;
						nbn.origin = nbn.tree = n.tree;
						nbn.active = true;
						active.push_back(nb->n);
					}
					else if ( nbn.tree != n.tree )
					{
						pathproc( p, *nb->n, min( min( n.maxflow, rflow[nb->link] ), nbn.maxflow ), nb->link );
						/*active.push_back(&p);
						n.active = true;*/
						break;
					}
				}
				if ( nb->last )
					break;
			}
		}
	}

	show(rflow, reinterpret_cast<Nod*>(node));
	DEBUG(	t.check();
			cout<<"\nmincut: "<<t.getDur();
			t.start();)
	//DEBUG(cout<<"done\nmarking...\n";)

	/*for ( Graph::arraySizeT i = 0; i < gr->nodenum(); ++i )
	{
		marking[i] = node[i].origin;
	}*/
	//mark((*gr)[0]);
	//mark((*gr)[1]);
	queue<const Graph::Node *> forMark;
	forMark.push(&(*gr)[0]);
	forMark.push(&(*gr)[1]);
	while ( !forMark.empty() )
	{
		const Graph::Node & c = *forMark.front();
		forMark.pop();
		for ( const Graph::Nbhd * nb = c.nb; ; ++nb )
		{
			if ( marking[c.place] != marking[nb->n->place] )
			if ( rflow[ nb->link ] > eps )
			{
				marking[nb->n->place] = marking[c.place];
				forMark.push(nb->n);//mark(*nb->n);
				//showm(marking);
			}
			if ( nb->last )
					break;
		}
	}
	DEBUG(	t.check();
			cout<<"\nmarking: "<<t.getDur();)
}

void GraphCut::killorigin(const Graph::Node & c)
{
	Node & n = node[c.place];
	n.origin = c.tag;
	if ( c.nb )
	{
		for ( const Graph::Nbhd * nb = c.nb; ; ++nb )
		{
			if ( node[nb->n->place].par == &c )
				killorigin(*nb->n);
			if ( nb->last )
					break;
		}
	}
}

void GraphCut::restoreorigin(const Graph::Node & c)
{
	Node & n = node[c.place], & pn = node[n.par->place];
	n.origin = pn.origin;
	n.maxflow = min( rflow[ n.parlink ], pn.maxflow );
	bool leaf = true;
	if ( c.nb )
	{
		for ( const Graph::Nbhd * nb = c.nb; ; ++nb )
		{
			Node & nbn = node[ nb->n->place ];
			if ( nbn.par == &c )
			{
				leaf = false;
				restoreorigin(*nb->n);
			}
			if ( nb->last )
					break;
		}
	}
	if ( leaf )
	if ( !n.active )
	{
		n.active = true;
		active.push_back(&c);
	}
}

void GraphCut::pathproc(const Graph::Node & p, const Graph::Node & q, const weightT flow, const arraySizeT link)
{
	queue<const Graph::Node *> orphan;
	const Graph::Node * t = &p;

	while ( node[t->place].par )
	{
		Node & n = node[t->place];
		const Graph::Node * c = t;
		t = n.par;
		n.maxflow -= flow;
		DEBUG(cout<<"loop 1 : "<<c->tag<<" -> "<<t->tag<<"\n";)
		if ( (rflow[n.parlink] -= flow) < eps )
		{
			orphan.push(c);
			n.par = nullptr;
			n.parlink = -1;
			killorigin(*c);
		}
	}
	t = &q;
	while ( node[t->place].par )
	{
		Node & n = node[t->place];
		const Graph::Node * c = t;
		t = n.par;
		n.maxflow -= flow;
		DEBUG(cout<<"loop 2 : "<<c->tag<<" -> "<<t->tag<<"\n";)
		if ( (rflow[n.parlink] -= flow) < eps )
		{
			orphan.push(c);
			n.par = nullptr;
			n.parlink = -1;
			killorigin(*c);
		}
	}
	rflow[link] -= flow;

	//DEBUG( cout<<p.place<<" to "<<q.place<<" | flow: "<<flow<<"\n"; )
	show(rflow, reinterpret_cast<Nod*>(node));

	while ( !orphan.empty() )
	{
		const Graph::Node * oph = orphan.front();
		orphan.pop();
		Node & n = node[oph->place];
		bool found = false;
		if ( oph->nb )
		{
			for ( const Graph::Nbhd * nb = oph->nb; ; ++nb )
			{
				Node & nbn = node[ nb->n->place ];
				if ( n.tree == nbn.tree )
				if ( rflow[ nb->link ] > eps )
				if ( nbn.origin == node[0].origin || nbn.origin == node[1].origin )
				{
					n.par = nb->n;
					n.parlink = nb->link;
					n.maxflow = min ( rflow[nb->link], nbn.maxflow );
					restoreorigin(*oph);
					found = true;
					if ( !n.active )
						active.push_back(oph);
					break;
				}
				if ( nb->last )
					break;
			}
		}
		if ( !found )
		{
			if ( oph->nb )
			{
				for ( const Graph::Nbhd * nb = oph->nb; ; ++nb )
				{
					Node & nbn = node[ nb->n->place ];
					if ( n.tree == nbn.tree )
					{
						if ( rflow[ nb->link ] > eps )
						{
							nbn.active = true;
							active.push_back(nb->n);
						}
						if ( oph == nbn.par )
						{
							orphan.push(nb->n);
							nbn.par = nullptr;
							nbn.parlink = -1;
						}
					}
					if ( nb->last )
					break;
				}
			}
			n.par = nullptr;
			n.parlink = -1;
			n.tree = 0;
			n.origin = 0;
			n.active = false;
			active.remove(oph);
		}
	}
}

/*void GraphCut::mark(const Graph::Node & c)
{
	if (c.nb)
	{
		for ( const Graph::Nbhd * nb = c.nb; ; ++nb )
		{
			if ( marking[c.place] != marking[nb->n->place] )
			if ( rflow[ nb->link ] > eps )
			{
				marking[nb->n->place] = marking[c.place];
				mark(*nb->n);
			}
			if ( nb->last )
					break;
		}
	}
}*/
