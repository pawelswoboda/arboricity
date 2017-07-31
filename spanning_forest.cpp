#include <stdio.h>
#include <stdlib.h>
#include "arboricity.h"

UndirectedGraph::SpanningForest::SpanningForest(int _n, Node* _nodes)
	: n(_n), nodes(_nodes)
{
	int i;
	parents = new Edge*[n];
	for (i=0; i<n; i++) parents[i] = NULL;
}

UndirectedGraph::SpanningForest::~SpanningForest()
{
	delete [] parents;
}

UndirectedGraph::NodeId UndirectedGraph::SpanningForest::FindLCA(Edge* e)
{
	NodeId i[2] = { e->i[0], e->i[1] }, p, q, r, LCA;
	int d = 0;

	nodes[i[0]].tmp_flag = nodes[i[1]].tmp_flag = 1;

	while ( 1 )
	{
		p = GetParent(i[d]);
		if (p < 0 || nodes[p].tmp_flag) break;
		nodes[p].tmp_flag = 1;
		i[d] = p;
		d ^= 1;
	}
	q = i[d ^ 1]; // >= 0

	if (p >= 0) // paths have intersected
	{
		LCA = p;
		r = e->i[d ^ 1];
		while (r != q)
		{
			nodes[r].tmp_flag = 0;
			r = GetParent(r);
		}
		nodes[q].tmp_flag = 0;
	}
	else
	{
		NodeId qq = q;
		while ( 1 )
		{
			q = GetParent(q);
			if (q < 0 || nodes[q].tmp_flag) break;
		}
		LCA = q;
		r = e->i[d ^ 1];
		while (r != qq)
		{
			nodes[r].tmp_flag = 0;
			r = GetParent(r);
		}
		nodes[qq].tmp_flag = 0;
	}

	r = e->i[d];
	while (r != p)
	{
		nodes[r].tmp_flag = 0;
		r = GetParent(r);
	}
	if (p >= 0) nodes[p].tmp_flag = 0;

	return LCA;
}

void UndirectedGraph::SpanningForest::RemoveEdge(Edge* e)
{
	NodeId p = e->i[0], q = e->i[1];
	if (parents[p] == e)
	{
		parents[p] = NULL;
	}
	else
	{
		assert(parents[q] == e);
		parents[q] = NULL;
	}
}

void UndirectedGraph::SpanningForest::AddEdge(Edge* e, NodeId p)
{
	NodeId q = p;
	while ( 1 )
	{
		NodeId q = GetParent(p);
		Edge* tmp = e; e = parents[p]; parents[p] = tmp;
		if (q < 0) break;
		p = q;
	}
}

void UndirectedGraph::SpanningForest::Print()
{
	NodeId k;
	printf("Forest:\n");
	for (k=0; k<n; k++)
	{
		NodeId p = GetParent(k);
		if (p >= 0) printf("    %d - %d\n", k, p);
		else        printf("    %d\n", k);
	}
}