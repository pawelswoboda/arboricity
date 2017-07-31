#include <stdio.h>
#include <stdlib.h>
#include "arboricity.h"
#include "maxflow-v3.04.src/graph.h"

UndirectedGraph::UndirectedGraph(int _node_num, int _user_edge_num_max)
	: node_num(_node_num), user_edge_num(0), edge_num(0), edge_num_max(2*_user_edge_num_max)
{
	int i;
	nodes = new Node[node_num+1];
	edges = new Edge[edge_num_max+node_num];
	for (i=0; i<=node_num; i++)
	{
		nodes[i].first_incoming = -1;
		nodes[i].tmp_flag = 0;
	}
	forests = NULL;
	compacting_mapping = NULL;
}

UndirectedGraph::~UndirectedGraph()
{
	delete [] nodes;
	delete [] edges;
	if (forests)
	{
		int f;
		for (f=0; f<forest_num; f++) delete forests[f];
		delete [] forests;
	}
	if (compacting_mapping) delete [] compacting_mapping;
}

UndirectedGraph::EdgeId UndirectedGraph::AddEdge(NodeId i, NodeId j, int weight)
{
	if (i<0 || i>=node_num || j<0 || j>=node_num || i==j || weight<0) { printf("Incorrect call to AddEdge()\n"); exit(1); }
	if (edge_num >= edge_num_max) { printf("too many edges\n"); exit(1); }

	Edge* e = &edges[edge_num ++];
	e->i[0] = i; e->i[1] = j; e->weight = weight;

	e = &edges[edge_num ++];
	e->i[0] = j; e->i[1] = i; e->weight = weight;

	return user_edge_num ++;
}

void UndirectedGraph::Compact()
{
	int _e;
	Edge* e;

	assert(!compacting_mapping && edge_num == 2*user_edge_num);

	compacting_mapping = new int[edge_num];

	for (_e=edge_num=0; _e<2*user_edge_num; _e++)
	{
		if (edges[_e].weight)
		{
			compacting_mapping[edge_num] = _e/2;
			e = &edges[edge_num];
			*e = edges[_e];
			e->next_incoming = nodes[e->i[1]].first_incoming;
			nodes[e->i[1]].first_incoming = edge_num;
			edge_num ++;
		}
	}
}

void UndirectedGraph::GetForestEdges(int k, EdgeId* forest)
{
	if (!compacting_mapping) { printf("Error: GetForestEdges() can only be called after Solve()\n"); exit(1); }
	if (k<0 || k>=forest_num) { printf("Error in GetForestEdges(): incorrect forest id\n"); exit(1); }

	NodeId i;
	SpanningForest* F = forests[k];

	for (i=0; i<node_num; i++)
	{
		if (!F->parents[i]) forest[i] = -1;
		else
		{
			int e = (int)(F->parents[i] - edges);
			forest[i] = (e < edge_num) ? compacting_mapping[e] : -1;
		}
	}
}

void UndirectedGraph::GetForestParents(int k, EdgeId* forest)
{
	if (!compacting_mapping) { printf("Error: GetForestParents() can only be called after Solve()\n"); exit(1); }
	if (k<0 || k>=forest_num) { printf("Error in GetForestParents(): incorrect forest id\n"); exit(1); }

	NodeId i;
	SpanningForest* F = forests[k];

	for (i=0; i<node_num; i++)
	{
		NodeId p = F->GetParent(i);
		forest[i] = (p < node_num) ? p : -1;
	}
}

void UndirectedGraph::PrintEGraph()
{
	int e;
	printf("EG graph:\n");
	for (e=0; e<edge_num+node_num; e++)
	{
		if (e==edge_num) printf("\n");
		printf("    %d -> %d (weight=%d)\n", edges[e].i[0], edges[e].i[1], edges[e].weight);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

int UndirectedGraph::Solve0()
{
	typedef Graph<int,int,int> G;

	G* g = new G(node_num + user_edge_num, user_edge_num);
	g->add_node(node_num + user_edge_num);

	int i, e;
	int weight_sum = 0;
	for (e=0; e<edge_num; e+=2)
	{
		weight_sum += edges[e].weight;
	}

	for (e=0; e<edge_num; e+=2)
	{
		g->add_tweights(e/2, edges[e].weight, 0);
		g->add_edge(e/2, edges[e].i[0] + user_edge_num, weight_sum, 0);
		g->add_edge(e/2, edges[e].i[1] + user_edge_num, weight_sum, 0);
	}

	int k0_min = 0, k0_max = weight_sum, k_prev = 0, k;

	while ( k0_min < k0_max )
	{
		////////////////////////
		////// choose k ////////
		////////////////////////
		k = k0_min + 2; // do this instead of binary search; with unit capacities may make more sense
		if (k >= k0_max-1) k = k0_max-1;

		/////////////////////////////////
		////// test where k0 > k ////////
		/////////////////////////////////
		for (i=0; i<node_num; i++)
		{
			g->add_tweights(i + user_edge_num, 0, k-k_prev);
		}
		k_prev = k;

		int f = g->maxflow();

		if (f < weight_sum) k0_min = k + 1; // k0 >  k
		else                k0_max = k;     // k0 <= k
	}

	// set Edge::weight
	G::arc_id a = g->get_first_arc();
	for (e=0; e<edge_num; e+=2)
	{
		a = g->get_next_arc(a); edges[e+1].weight = g->get_rcap(a); a = g->get_next_arc(a);
		a = g->get_next_arc(a); edges[e  ].weight = g->get_rcap(a); a = g->get_next_arc(a);
	}
	delete g;

	return k0_min;
}

void UndirectedGraph::ComputeSourceCapacities(int k)
{
	int i, e;
	for (i=0; i<node_num; i++)
	{
		edges[i + edge_num].weight = k;
	}
	for (e=0; e<edge_num; e++)
	{
		edges[edges[e].i[1] + edge_num].weight -= edges[e].weight;
	}
}

bool UndirectedGraph::CheckWithMaxflow(int k)
{
	typedef Graph<int,int,int> G;

	G* g = new G(node_num, edge_num);
	g->add_node(node_num);

	int i, e;

	ComputeSourceCapacities(k);
	for (i=0; i<node_num; i++)
	{
		g->add_tweights(i, edges[i + edge_num].weight, 0);
	}

	for (e=0; e<edge_num; e++)
	{
		g->add_edge(edges[e].i[0], edges[e].i[1], edges[e].weight, 0);
	}
	g->maxflow();

	for (i=0; i<node_num; i++)
	{
		g->add_tweights(i, 0, k);
		g->mark_node(i);
		int f = g->maxflow(true);
		if (f < k)
		{
			delete g;
			return false;
		}
		g->add_tweights(i, 0, -k);
		g->mark_node(i);
	}

	delete g;
	return true;
}

int UndirectedGraph::Solve()
{
	int i, k0, k;

	k0 = Solve0();
	printf(" k0=%d ", k0);

	Compact();

	// add edges from the source (=node_num) to every node
	for (i=0; i<node_num; i++)
	{
		edges[i + edge_num].i[0] = node_num;
		edges[i + edge_num].i[1] = i;

		edges[i + edge_num].next_incoming = nodes[i].first_incoming;
		nodes[i].first_incoming = i + edge_num;
	}

#ifdef PRINT_DEBUG
	PrintEGraph();
#endif

	int k_check = -1;
	if ( 0 ) // sanity check
	{
		for (k_check=k0; ; k_check++)
		{
			if (CheckWithMaxflow(k_check)) break;
		}
	}

	ComputeSourceCapacities(k0);
	k = ComputeForests(k0);

	if (k_check>=0 && k_check != k)
	{
		printf("Error: Different answers!\n");
		exit(1);
	}

	printf(" k=%d\n", k);

	return k;
}
