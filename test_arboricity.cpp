#include <stdio.h>
#include <stdlib.h>
#include "arboricity.h"

struct Problem
{
	Problem(int _n, int _m_max) : n(_n), m(0), m_max(_m_max)
	{
		edges = new Edge[m_max];
	}
	~Problem()
	{
		delete [] edges;
	}
	void AddEdge(int i, int j, int w)
	{
		if (m >= m_max) { printf("Too many edges\n"); exit(1); }
		edges[m].i = i;
		edges[m].j = j;
		edges[m].w = w;
		m ++;
	}
	void GenerateRandom()
	{
		m = 0;
		while ( m < m_max )
		{
			int i = rand() % n;
			int j = rand() % (n-1);
			if (j>=i) j ++;
			int w = 1 + (rand() % 2);
			AddEdge(i, j, w);
		}
	}

	struct Edge
	{
		int i, j; // node id's
		int w;
	};
	int n, m, m_max; // # nodes, # edges

	Edge* edges;
};

struct GridGraph : public Problem
{
  GridGraph(int n, int m) : 
    Problem(n*m, (n-1)*m + n*(m-1))
  {
    // horizontal edges
    for(int i=0; i<n-1; ++i) {
      for(int j=0; j<m; ++j) {
        this->AddEdge( i*m + j, (i+1)*m + j, 1);
      }
    }
    // vertical edges
    for(int i=0; i<n; ++i) {
      for(int j=0; j<m-1; ++j) {
        this->AddEdge( i*m + j, i*m + j+1, 1);
      }
    }
  } 
};

struct Torus : public Problem
{
  Torus(int n, int m)
    : Problem(n*m, 2*n*m)
  {
    // horizontal edges
    for(int i=0; i<n; ++i) {
      for(int j=0; j<m; ++j) {
        this->AddEdge( i*m + j, ((i+1)%n)*m + j, 1);
      }
    }
    // vertical edges
    for(int i=0; i<n; ++i) {
      for(int j=0; j<m; ++j) {
        this->AddEdge( i*m + j, i*m + ((j+1)%m), 1);
      }
    }
  } 
};

void TestDecomposition(Problem* P)
{
	int i, e, k;
	UndirectedGraph* g = new UndirectedGraph(P->n, P->m);
	for (e=0; e<P->m; e++) g->AddEdge(P->edges[e].i, P->edges[e].j, P->edges[e].w);
	int forest_num = g->Solve();

	if ( 0 ) // test whether a valid solution. O(n^2) complexity.
	{
		int* forest = new int[P->n];
		for (k=0; k<forest_num; k++)
		{
			g->GetForestEdges(k, forest);
			for (i=0; i<P->n; i++)
			{
				e = forest[i];
				if (e >= 0)
				{
					if (e >= P->m || (P->edges[e].i!=i && P->edges[e].j!=i)) { printf("Error: incorrect edge id\n"); exit(1); }
					P->edges[e].w --;
				}

				// trace the path towards a root; if there are cycles, will not terminate
				int j = i;
				while ( 1 )
				{
					e = forest[j];
					if (e < 0) break;
					j = (P->edges[e].i == j) ? P->edges[e].j : P->edges[e].i;
				}
			}
		}

		for (e=0; e<P->m; e++)
		{
			if (P->edges[e].w != 0) { printf("Error: incorrect covering\n"); exit(1); }
		}

		delete [] forest;
	}

	delete g;
}


int main()
{
  // test grid graphs, arboricity should be two
  printf("Testing grid graphs\n");
  for(int n=100; n<=1000; n+=100) {
    for(int m=100; m<=1000; m+=100) {
      GridGraph G(n,m);
      TestDecomposition(&G);
    }
  }

  // test toroidal graphs, arboricity should be three
  printf("Testing toroidal graphs\n");
  for(int n=100; n<=1000; n+=100) {
    for(int m=100; m<=1000; m+=100) {
      Torus G(n,m);
      TestDecomposition(&G);
    }
  }

  // random graphs
  printf("Testing random graphs\n"); 
	int n = 10, m = 50;
	Problem P(n, m);
	int seed;
	for (seed=4; seed<1000; seed++)
	{
		printf("seed=%d\n", seed);
		srand(seed);
		P.GenerateRandom();
		TestDecomposition(&P);
	}
}
