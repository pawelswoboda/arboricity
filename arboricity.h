#ifndef __ARBORESCENSE_H__
#define __ARBORESCENSE_H__

#include <string.h>
#include <assert.h>


//#define PRINT_DEBUG

class UndirectedGraph
{
public:
	typedef int NodeId;
	typedef int EdgeId;

	UndirectedGraph(int node_num, int user_edge_num_max);
	~UndirectedGraph();

	EdgeId AddEdge(NodeId i, NodeId j, int weight); // the first call returns 0, the second returns 1, and so on.
	                                                // In the decomposition this edge must be covered by 'weight' forests

	int Solve(); // returns the number of forests 'forest_num'.
	             // After calling Solve(), the k'th forest (for k\in[0,forest_num-1]) can be obtained by one of the two functions below.

	void GetForestEdges(int k, EdgeId* forest); // 'forest' must be an array of size 'node_num' allocated by the user
	                                            // interpretation of the result:
	                                            //   if forest[i]< 0 then node i is a root
	                                            //   if forest[i]>=0 then forest[i] is the id of the parent edge of i. (One of the endpoints of this edge is i).
	                                            // Note, forest[i] \in [-1,edge_num-1]
	void GetForestParents(int k, EdgeId* forest); // similar to the previous function, but forest[i] is the id of the parent node, not of the parent edge.
	                                              // Note, forest[i] \in [-1,node_num-1]


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	struct Node;
	struct Edge;

	struct Node
	{
		EdgeId first_incoming;
		int incoming; // number of incoming edges in spanning forests
		int tmp_flag : 1; // must be kept to 0
	};
	struct Edge
	{
		NodeId i[2];
		EdgeId next_incoming;
		int weight;

#define PP_NO_PARENT            -2
#define PP_PARENT_SAME_ENDPOINT -1
		int path_parent;  // if >= 0 then gives the forest id 
		Edge* path_parent_edge;
	};
	struct SpanningForest
	{
		// n = node_num+1, m = edge_num+node_num
		SpanningForest(int n, Node* nodes);
		~SpanningForest();
		NodeId GetParent(NodeId i)
		{
			Edge* e = parents[i];
			if (!e) return -1;
			return e->i[ (e->i[0] == i) ? 1 : 0 ];
		}
		bool isPresent(Edge* e) { return (parents[e->i[0]] == e || parents[e->i[1]] == e); }
		void Print();

		// in the first two functions e must not be in the forest, in the last one e must be in the forest.
		NodeId FindLCA(Edge* e); // returns lowest common ancestor of e->i[0] and e->i[1] (or -1, if in different trees).
		void AddEdge(Edge* e, NodeId p); // p is an endpoint of e; the root of the component to which p belongs will be changed.
		void RemoveEdge(Edge* e);

		int n;
		Node* nodes; // of size n. Only need to access Node::tmp_flag
		Edge** parents; // of size n
	};

	int node_num, user_edge_num, edge_num, edge_num_max; // in the beginning edge_num=2*user_edge_num (since each edge is duplicated
	                                                     // - backward and forward copies are created. Once all edges have been added,
	                                                     // zero-weight edges are removed, and edge_num is possibly reduced.
	Node* nodes; // of size node_num + 1
	Edge* edges; // of size edge_num_max + node_num

	int* compacting_mapping;

	SpanningForest** forests;
	int forest_num;

	struct Queue
	{
		Queue(int size_max_estimated=4)
		{
			if (size_max_estimated < 4) size_max_estimated = 4;
			queue = (Edge**) malloc(size_max_estimated*sizeof(Edge*));
			queue_end_allocated = queue + size_max_estimated;
		}
		~Queue() { free(queue); }
		void Init() { queue_start = queue_end = queue; }
		void Restart() { queue_start = queue; }
		void Add(Edge* e)
		{
			if (queue_end >= queue_end_allocated)
			{
				int a = (int)(queue_start - queue);
				int b = (int)(queue_end - queue);
				int size_max = 2*b;
				queue = (Edge**) realloc(queue, size_max*sizeof(Edge*));
				queue_start = queue + a;
				queue_end = queue + b;
				queue_end_allocated = queue + size_max;
			}
			*queue_end ++ = e;
		}
		Edge* Remove() { return (queue_start == queue_end) ? NULL : (*queue_start ++); }
	private:
		Edge** queue;
		Edge** queue_start;
		Edge** queue_end;
		Edge** queue_end_allocated;
	};

	Queue queue;

	int Solve0();
	void Compact(); // deletes edges with zero weight, allocates compacting_mapping

	// all functions below can be called only after Solve0() and Compact()

	void ComputeSourceCapacities(int k);
	bool CheckWithMaxflow(int k); // must have k >= k0
	                              // returns true if k_opt <= k
	                              // Not optimized, just for testing. (Ideally, should be Hao-Orlin algorithm)
	int ComputeForests(int k); // must have k \in [k0, k_opt]
	Edge* Search(NodeId z);
	void Augment(Edge* e);

	void PrintEGraph();
};

#endif
