#include <stdio.h>
#include <stdlib.h>
#include "arboricity.h"


int UndirectedGraph::ComputeForests(int k_current)
{
	int i;
	Edge* e;

	forests = new SpanningForest*[k_current];
	forest_num = 0;

	for (i=0; i<=node_num; i++)
	{
		nodes[i].incoming = 0;
	}

	queue.Init();
	for (e=edges; e<edges+edge_num+node_num; e++) e->path_parent = PP_NO_PARENT;

	while ( 1 )
	{
		if (forest_num == k_current)
		{
			for (i=0; i<node_num; i++) { if (nodes[i].incoming < forest_num) break; }
			if (i == node_num) return forest_num;
			k_current ++;
			for (i=0; i<node_num; i++) edges[i+edge_num].weight ++;
			SpanningForest** forests_old = forests;
			forests = new SpanningForest*[k_current];
			memcpy(forests, forests_old, forest_num*sizeof(SpanningForest*));
			delete [] forests_old;
		}

		forests[forest_num ++] = new SpanningForest(node_num + 1, nodes);

		bool augmentation;
		do
		{
			augmentation = false;
			for (i=0; i<node_num; i++)
			{
				if (nodes[i].incoming < forest_num)
				{
					Edge* e = Search(i);
					if (e)
					{
						Augment(e);
						augmentation = true;
					}
				}
			}
		} while (augmentation);
	}
}

void UndirectedGraph::Augment(Edge* e_last)
{
#ifdef PRINT_DEBUG
	printf("augmenting path in reverse:");
#endif

	Edge* e = e_last;
	Edge* e_next = NULL;
	while ( 1 )
	{
#ifdef PRINT_DEBUG
		printf(" (%d,%d)", e->i[0], e->i[1]);
#endif
		Edge* e_prev = e->path_parent_edge;
		e->path_parent_edge = e_next;
		e_next = e;
		if (!e_prev) break;
		e = e_prev;
	}
#ifdef PRINT_DEBUG
	printf("\n");
#endif

	while ( e != e_last )
	{
		Edge* g = e;
		e = e->path_parent_edge;
		int k = e->path_parent;
		if (k >= 0)
		{
			if (!forests[k]->isPresent(e))
			{
				printf("Error #1 in Augment()\n");
				exit(1);
			}
			if (forests[k]->isPresent(g))
			{
				printf("Error #2 in Augment()\n");
				exit(1);
			}

			forests[k]->RemoveEdge(e);
			e->weight ++;
			nodes[e->i[1]].incoming --;

			forests[k]->AddEdge(g, g->i[0]);
			g->weight --;
			nodes[g->i[1]].incoming ++;
		}
	}

	int k;
	for (k=forest_num-1; k>=0; k--)
	{
		if (forests[k]->isPresent(e)) continue;
		NodeId LCA = forests[k]->FindLCA(e);
		if (LCA < 0)
		{
			forests[k]->AddEdge(e, e->i[0]);
			e->weight --;
			nodes[e->i[1]].incoming ++;
			break;
		}
	}

#ifdef PRINT_DEBUG
	for (k=0; k<forest_num; k++)
	{
		forests[k]->Print();
	}
#endif
}

UndirectedGraph::Edge* UndirectedGraph::Search(NodeId z)
{
	int _e;
	Edge* e;
	Edge* g;
	int k, d, p;

	queue.Restart();
	while ((e=queue.Remove())) e->path_parent = PP_NO_PARENT;
	queue.Init();

	for (_e=nodes[z].first_incoming; _e>=0; _e=edges[_e].next_incoming)
	{
		e = &edges[_e];
		if (e->weight)
		{
			e->path_parent = PP_PARENT_SAME_ENDPOINT;
			e->path_parent_edge = NULL;
			queue.Add(e);
		}
	}

	while ((e=queue.Remove()))
	{
		for (k=0; k<forest_num; k++)
		{
			SpanningForest* F = forests[k];
			if (!F->isPresent(e))
			{
				NodeId LCA = F->FindLCA(e);
				if (LCA < 0)
				{
					return e; // success - found joining edge
				}
				for (d=0; d<2; d++)
				{
					for (p=e->i[d]; p != LCA; p=F->GetParent(p))
					{
						Edge* g = F->parents[p];
						if (g->path_parent == PP_NO_PARENT)
						{
							g->path_parent = k;
							g->path_parent_edge = e;
							queue.Add(g);
						}
					}
				}
			}
			p = e->i[1];
			for (_e=nodes[p].first_incoming; _e>=0; _e=edges[_e].next_incoming)
			{
				g = &edges[_e];
				if (g->weight && g->path_parent == PP_NO_PARENT)
				{
					g->path_parent = PP_PARENT_SAME_ENDPOINT;
					g->path_parent_edge = e;
					queue.Add(g);
				}
			}
		}
	}

	return NULL;
}
